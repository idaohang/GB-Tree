/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 4/2/2014
 */
#ifndef GBTENGINE_H_
#define GBTENGINE_H_

#include <vector>
#include <stdio.h>
#include <string>
#include "GBTreeBase.h"
/**
 * the class that takes, parses, and executes the user commands.
 */
class GBTEngine {
 public:
    
  /**
   * executes a SELECT statement.
   * all conditions in conds must be ANDed together.
   * the result of the SELECT is printed on screen.
   * @param attr[IN] attribute in the SELECT clause
   * (1: key, 2: value, 3: *, 4: count(*))
   * @param table[IN] the table name in the FROM clause
   * @param conds[IN] list of conditions in the WHERE clause
   * @return error code. 0 if no error
   */
  static RT EqualSelect(const std::string& table, double longitude, double latitude, std::string& value);

  static RT RangeSelect(const std::string table, double* lnglat, std::vector<std::string>& values);

  /**
   * load a table from a load file.
   * @param table[IN] the table name in the LOAD command
   * @param loadfile[IN] the file name of the load file
   * @param index[IN] true if "WITH INDEX" option was specified
   * @return error code. 0 if no error
   */
  static RT load(const std::string& table, const std::string& loadfile, bool index);

  /**
   * parse a line from the load file into the (key, value) pair.
   * @param line[IN] a line from a load file
   * @param key[OUT] the key field of the tuple in the line
   * @param value[OUT] the value field of the tuple in the line
   * @return error code. 0 if no error
   */
  static RT parseLoadLine(char* line, double& lng, double& lat, std::string& value);
 private:
  static RT EqualSelectImpl(const std::string table, double longitude, double latitude, std::string& value);
  static RT RangeSelectImpl(const std::string table, double* lnglat, std::vector<std::string>& values);
};


#endif
