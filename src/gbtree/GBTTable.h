/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 4/2/2014
 */
#ifndef GBTTABLE_H_
#define GBTTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "../storagemanager/GBTFile.h"
#include "../base/GBTreeBase.h"
/**
 * The data structure for pointing to a particular record in a GBTTable.
 * A record id consists of pid (PageId) and sid (the slot number in the page)
 */
typedef struct {
  PageId  pid;  // page number. the first page is 0
  int     sid;  // slot number. the first slot is 0
} RecordId;

//
// helper functions for RecordId
// 

// RecordId iterators
RecordId& operator++ (RecordId& rid);
RecordId  operator++ (RecordId& rid, int);

// RecordId comparators
bool operator> (const RecordId& r1, const RecordId& r2);
bool operator< (const RecordId& r1, const RecordId& r2);
bool operator>= (const RecordId& r1, const RecordId& r2);
bool operator<= (const RecordId& r1, const RecordId& r2);
bool operator== (const RecordId& r1, const RecordId& r2);
bool operator!= (const RecordId& r1, const RecordId& r2);

/**
 * read/write a record to a table file
 */
class GBTTable {
 public:

  // maximum length of the value field
  static const int MAX_VALUE_LENGTH = 100;  

  // number of record slots per page
  static const int RECORDS_PER_PAGE = (GBTFile::PAGE_SIZE - sizeof(int))/ (sizeof(uint64_t) + MAX_VALUE_LENGTH);  
    // Note that we subtract sizeof(int) from PAGE_SIZE because the first
    // four bytes in the page is used to store # records in the page.

  GBTTable();
  GBTTable(const std::string& filename, char mode);
  
  /**
   * open a file in read or write mode.
   * when opened in 'w' mode, if the file does not exist, it is created.
   * @param filename[IN] the name of the file to open
   * @param mode[IN] 'r' for read, 'w' for write
   * @return error code. 0 if no error
   */
  RT open(const std::string& filename, char mode);

  /**
   * close the file.
   * @return error code. 0 if no error
   */
  RT close();

  /**
   * read a record from the file. note that every record is a (key, value) pair.
   * @param rid[IN] the id of the record to read
   * @param key[OUT] the record key
   * @param value[OUT] the record valu
   * @return error code. 0 if no error
   */
  RT read(const RecordId& rid, uint64_t& key, std::string& value) const;

  /**
   * append a new record at the end of the file.
   * note that GBTTable does not have write() function.
   * append is the only way to write a record to a GBTTable.
   * @param key[IN] the record key
   * @param value[IN] the record value
   * @param rid[OUT] the location of the stored record
   * @return error code. 0 if no error
   */
  RT append(uint64_t key, const std::string& value, RecordId& rid);

  /**
   * note the +1 part. The rid of the last record is endRid()-1.
   * @return (last record id + 1) of the GBTTable
   */
  const RecordId& endRid() const;

 private:
  GBTFile pf;     // the GBTFile used to store the records
  RecordId erid;   // the last record id of the file + 1
};

#endif
