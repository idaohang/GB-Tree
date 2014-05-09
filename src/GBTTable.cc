/*
 * =====================================================================================
 *
 *       Filename:  GBTTable.cc
 *
 *    Description:  read and write record into table file
 *
 *        Version:  1.0
 *        Created:  04/02/2014 04:54:25 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include "GBTTable.h"

using std::string;

//
// helper functions for page manipultation
//

// compute the pointer to the n'th slot in a page
static char* slotPtr(char* page, int n);

// read the record in the n'th slot in the page
static void readSlot(const char* page, int n, uint64_t& key, std::string& value);

// write the record to the n'th slot in the page
static void writeSlot(char* page, int n, uint64_t key, const std::string& value);

// get # records stored in the page
static int getRecordCount(const char* page);

// update # records stored in the page
static void setRecordCount(char* page, int count);


//
// helper functions for RecordId manipulation
//

// postfix record id iterator
RecordId operator++ (RecordId& rid, int)
{
  RecordId prid(rid);

  // if the end of a page is reached, move to the next page
  if (++rid.sid >= GBTTable::RECORDS_PER_PAGE) {
    rid.pid++;
    rid.sid = 0;
  }

  return prid;
}

// prefix record id iterator
RecordId& operator++ (RecordId& rid)
{
  // if the end of a page is reached, move to the next page
  if (++rid.sid >= GBTTable::RECORDS_PER_PAGE) {
    rid.pid++;
    rid.sid = 0;
  } 

  return rid;
}

// RecordId comparators
bool operator < (const RecordId& r1, const RecordId& r2)
{
  if (r1.pid < r2.pid) return true;
  if (r1.pid > r2.pid) return false;
  return (r1.sid < r2.sid);
}

bool operator > (const RecordId& r1, const RecordId& r2)
{
  if (r1.pid > r2.pid) return true;
  if (r1.pid < r2.pid) return false;
  return (r1.sid > r2.sid);
}

bool operator <= (const RecordId& r1, const RecordId& r2)
{
  if (r1.pid < r2.pid) return true;
  if (r1.pid > r2.pid) return false;
  return (r1.sid <= r2.sid);
}

bool operator >= (const RecordId& r1, const RecordId& r2)
{
  if (r1.pid > r2.pid) return true;
  if (r1.pid < r2.pid) return false;
  return (r1.sid >= r2.sid);
}

bool operator == (const RecordId& r1, const RecordId& r2)
{
  return ((r1.pid == r2.pid) && (r1.sid == r2.sid));
}

bool operator != (const RecordId& r1, const RecordId& r2)
{
  return ((r1.pid != r2.pid) || (r1.sid != r2.sid));
}


GBTTable::GBTTable()
{
  erid.pid = 0;
  erid.sid = 0;
}

GBTTable::GBTTable(const string& filename, char mode)
{
  open(filename, mode);
}

RT GBTTable::open(const string& filename, char mode)
{
  RT   rc;
  char page[GBTFile::PAGE_SIZE];

  // open the page file
  if ((rc = pf.open(filename, mode)) < 0) return rc;
  
  //
  // in the rest of this function, we set the end record id
  //

  // get the end pid of the file
  erid.pid = pf.endPid();

  // if the end pid is zero, the file is empty.
  // set the end record id to (0, 0).
  if (erid.pid == 0) {
    erid.sid = 0;
    return 0;
  }

  // obtain # records in the last page to set sid of the end record id.
  // read the last page of the file and get # records in the page.
  // remeber that the id of the last page is endPid()-1 not endPid().
  if ((rc = pf.read(--erid.pid, page)) < 0) {
    // an error occurred during page read
    erid.pid = erid.sid = 0;
    pf.close();
    return rc;
  }

  // get # records in the last page
  erid.sid = getRecordCount(page);
  if (erid.sid >= RECORDS_PER_PAGE) {
    // the last page is full. advance the end record id to the next page.
    erid.pid++;
    erid.sid = 0;
  }
  
  return 0;
}

RT GBTTable::close()
{
  erid.pid = 0;
  erid.sid = 0;

  return pf.close();
}

RT GBTTable::read(const RecordId& rid, uint64_t& key, string& value) const
{
  RT   rc;
  char page[GBTFile::PAGE_SIZE];
  
  // check whether the rid is in the valid range
  if (rid.pid < 0 || rid.pid > erid.pid) return RT_INVALID_RID;
  if (rid.sid < 0 || rid.sid >= GBTTable::RECORDS_PER_PAGE) return RT_INVALID_RID;
  if (rid >= erid) return RT_INVALID_RID;
  
  // read the page containing the record
  if ((rc = pf.read(rid.pid, page)) < 0) return rc;

  // read the record from the slot in the page
  readSlot(page, rid.sid, key, value);

  return 0;
}

RT GBTTable::append(uint64_t key, const std::string& value, RecordId& rid)
{
  RT   rc;
  char page[GBTFile::PAGE_SIZE];

  // unless we are writing to the the first slot of an empty page,
  // we have to read the page first
  if (erid.sid > 0) {
    if ((rc = pf.read(erid.pid, page)) < 0) return rc;
  } else {
    // if this is the first slot of an empty page
    // we can simply initialize the page with zeros
    memset(page, 0, GBTFile::PAGE_SIZE);
  }
    
  // write the record to the first empty slot 
  writeSlot(page, erid.sid, key, value);

  // the first four bytes in the page stores # records in the page.
  // update this number.
  setRecordCount(page, erid.sid + 1);

  // write the page to the disk
  if ((rc = pf.write(erid.pid, page)) < 0) return rc;
    
  // we need to output the rid of the record slot
  rid = erid;

  // advance the end record id by one to the next empty slot
  ++erid;

  return 0;
}

const RecordId& GBTTable::endRid() const
{
  return erid;
}

static int getRecordCount(const char* page)
{
  int count;

  // the first four bytes of a page contains # records in the page
  memcpy(&count, page, sizeof(int));
  return count;
}

static void setRecordCount(char* page, int count)
{
  // the first four bytes of a page contains # records in the page
  memcpy(page, &count, sizeof(int));
}

static char* slotPtr(char* page, int n) 
{
  // compute the location of the n'th slot in a page.
  // remember that the first four bytes in a page is used to store
  // # records in the page and each slot consists of an integer and
  // a string of length MAX_VALUE_LENGTH
  return (page+sizeof(int)) + (sizeof(uint64_t)+GBTTable::MAX_VALUE_LENGTH)*n;
}

static void readSlot(const char* page, int n, uint64_t& key, std::string& value)
{
  // compute the location of the record
  char *ptr = slotPtr(const_cast<char*>(page), n);

  // read the key 
  memcpy(&key, ptr, sizeof(uint64_t));

  // read the value
  value.assign(ptr + sizeof(uint64_t));
}

static void writeSlot(char* page, int n, uint64_t key, const std::string& value)
{
  // compute the location of the record
  char *ptr = slotPtr(page, n);

  // store the key
  memcpy(ptr, &key, sizeof(uint64_t));

  // store the value. 
  if ((int)value.size() >= GBTTable::MAX_VALUE_LENGTH) {
    // when the string is longer than MAX_VALUE_LENGTH, truncate it.
    memcpy(ptr + sizeof(uint64_t), value.c_str(), GBTTable::MAX_VALUE_LENGTH -1);
    *(ptr + sizeof(uint64_t) + GBTTable::MAX_VALUE_LENGTH - 1) = 0;
  } else {
    strcpy(ptr + sizeof(uint64_t), value.c_str());
  }
}
