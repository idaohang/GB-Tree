/*
 * =====================================================================================
 *
 *       Filename:  Distance.cc
 *
 *    Description:  Distance calculation
 *
 *        Version:  1.0
 *        Created:  05/21/2014 04:02:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include <cmath>
#include "Distance.h"
static const double PI = 3.1415926;
static const double EARTH_RADIUS = 6378.137;
/* *
 * convert the latitude and longitude to radian.
 *
 * */
static double Rad(double d)
{
	return d * PI / 180.0 ;
}
double LatLon2Dist(double lat1, double lng1, double lat2, double lng2)
{
	double radLat1 = Rad(lat1);
	double radLat2 = Rad(lat2);
	double first = radLat1 - radLat2;
	double second = Rad(lng1) - Rad(lng2);

	double s = 2 * asin(sqrt(pow(sin(first/2), 2) + cos(radLat1) * cos(radLat2)
				* pow( sin(second / 2), 2 )));
	s *= EARTH_RADIUS;
	s *= 1000;
	return s;
}

double FlatDistance(double* low, double* high, uint32_t dimension)
{
	uint32_t i;
	double ret = 0.0;
	double tmp = 0.0;
	for(i = 0; i < dimension; ++i)
	{
		tmp = std::abs(high[i] - low[i]);
		ret += tmp * tmp;
	}
	return sqrt(ret);
}
