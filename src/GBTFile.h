/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 4/2/2014
 */
#ifndef GBTFILE_H_
#define GBTFILE_H_

#include "GBTreeBase.h"
#include <string>

typedef int PageId;

/**
 * read/write a file in the unit of a page
 */
class GBTFile {
 public:

  static const int PAGE_SIZE = 16384;    // the size of a page is 1KB

  GBTFile();
  GBTFile(const std::string& filename, char mode);

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
   * read a disk page into memory buffer.
   * @param pid[IN] the page to read
   * @param buffer[OUT] pointer to memory buffer
   * @return error code. 0 if no error
   */
  RT read(PageId pid, void *buffer) const;
  
  /**
   * write the memory buffer to the disk page.
   * if (pid >= endPid()), the file is expanded such that
   * endPid() becomes (pid + 1).
   * @param pid[IN] page to write to
   * @param buffer[IN] the content to write
   * @return error code. 0 if no error
   */
  RT write(PageId pid, const void *buffer);
    
  /**
   * note the +1 part. The last page id in the file is actually endPid()-1.
   * that is, the last page can be read by "read(endPid()-1, buffer)".
   * @return the id of the last page in the file (+ 1)
   */
  PageId endPid() const;

  /**
   * @return the total # of disk reads
   */
  static int getPageReadCount()  { return readCount; }
  
  /**
   * @return the total # of disk writes
   */
  static int getPageWriteCount() { return writeCount; }

 protected:
  /**
   * move the file cursor to the beginning of a page.
   * this is an internal function not exposed to public.
   * @param pid[IN] page to seek to
   * @return error code. 0 if no error
   */
  RT seek(PageId pid) const;

 private:
  int     fd;     // file descriptor of the associated unix file
  PageId  epid;   // (last page id + 1) of the file

  //
  // the following set of members implement LRU caching 
  //
  static const int CACHE_COUNT = 10;

  static int cacheClock; // clock tick counter for LRU policy

  // the actual cache data structure
  static struct cacheStruct {
    int    fd;              // file id of the cached page
    PageId pid;             // page id of the cached page
    int    lastAccessed;    // the last time the cached page was accessed
                            //   (lastAccessed == 0) means that the buffer is empty
    char buffer[PAGE_SIZE]; // the buffer used for caching
  } readCache[CACHE_COUNT];

  static int readCount;  // total # of page reads 
  static int writeCount; // total # of page writes 
};

#endif
