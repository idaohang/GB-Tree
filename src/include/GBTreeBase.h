/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 4/2/2014
 */
#ifndef GBTREEBASE_H_
#define GBTREEBASE_H_
//#define DEBUG
#include <string.h>

#if defined(_MSC_VER) && (_MSC_VER <= 1500)
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int64 uint64_t;
#define UINT64_C(C) ((uint64_t) C ## ULL)
#else
#define __STDC_CONSTANT_MACROS 1
#include <stdint.h>
#endif

typedef int RT;

const double DOUBLE_EPSILON = 0.00000001;

const uint32_t DATA_BIT_PRECISION = 64;

const int RT_FILE_OPEN_FAILED    = -1001;
const int RT_FILE_CLOSE_FAILED   = -1002;
const int RT_FILE_SEEK_FAILED    = -1003;
const int RT_FILE_READ_FAILED    = -1004;
const int RT_FILE_WRITE_FAILED   = -1005;
const int RT_INVALID_FILE_MODE   = -1006;
const int RT_INVALID_PID         = -1007;
const int RT_INVALID_RID         = -1008;
const int RT_INVALID_FILE_FORMAT = -1009;
const int RT_NODE_FULL           = -1010;
const int RT_INVALID_CURSOR      = -1011;
const int RT_NO_SUCH_RECORD      = -1012;
const int RT_END_OF_TREE         = -1013;
const int RT_INVALID_ATTRIBUTE   = -1014;
const int RT_INVALID_NODE		  = -1015;
const int RT_MEMCPY_ERROR        = -1016;
const int RT_END_OF_NODE         = -1017;
const int RT_DUPLICATE_FULL      = -1018;
const int RT_GEOHASH_ERROR		  = -1030;
const int RT_GEOQUERY_INVALID_RANGE = -1040;

#endif
