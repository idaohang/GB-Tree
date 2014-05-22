/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
//declare the types to adapt to the windows and linux
#ifndef TOOLS_H_
#define TOOLS_H_
#if (defined(_MSC_VER) || defined _WIN32 || defined _WIN64) && !defined __GNUC__
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define UINT64_C(C) ((uint64_t) C ## ULL)
#else
#define __STDC_CONSTANT_MACROS 1
#include <stdint.h>
#endif

//declare the keyword to generate the shared library. 
#if (defined(_MSC_VER) || defined _WIN32 || defined _WIN64) && !defined __GNUC__
#ifdef INDEX_CREATE_SHARED_LIB
#define IDX_SO_ __declspec(dllexport)
#else
#define IDX_SO_ __declspec(dllimport)
#endif
#else
#define IDX_SO_
#endif


#endif
