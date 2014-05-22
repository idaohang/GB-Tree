/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
#ifndef DISTANCE_H_
#define DISTANCE_H_

#include "Tools.h"
/* *
 * calculate the distance of longtitude and latitude at the WGS-84 coordinate system
 * */
extern double LatLon2Dist(double lat1, double lng1, double lat2, double lng2);

/* *
 * calculate the distance on flat
 * */
extern double FlatDistance(double* low, double* high, uint32_t dimension);

#endif
