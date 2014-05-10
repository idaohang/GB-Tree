/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
#ifndef TEST_GEOQUERY_H_
#define TEST_GEOQUERY_H_

/* *
 * Test the point select.
 * */
int TestPointSelect(const char* table_name, const char*data_file);
/* *
 * Test the range query
 * */
int TestRangeQuery(const char* table_name, const char*data_file);
/* *
 * Test the Nearest query
 * */
int TestNearestQuery(const char* table_name, const char*data_file);

#endif

