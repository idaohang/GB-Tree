/*
 * =====================================================================================
 *
 *       Filename:  TestGeoQuery.cc
 *
 *    Description:  Test
 *
 *        Version:  1.0
 *        Created:  04/17/2014 10:26:44 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include <assert.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "TestGeoQuery.h"
#include "GBTEngine.h"


int TestPointSelect()
{
	std::string value;
	std::string table("zj");
	std::string loadfile("data//zj.txt");
	GBTEngine::load(table, loadfile, true);
	GBTEngine::EqualSelect(table, 120.9349752,30.84378421, value);
	assert(value.compare("亚细亚饭店") == 0);
	GBTEngine::EqualSelect(table, 120.1656219,30.19457983, value);
	assert(value.compare("空港票务滨江售票处") == 0);
	GBTEngine::EqualSelect(table, 120.1272897,30.34521857, value);
	assert(value.compare("祥运路") == 0);
	return 0;
}
int TestRangeQuery()
{
	int rt;
	std::string value;
	std::string table("zj");
	std::string loadfile("data/zj.txt");
	std::vector<std::string> outputs;
	rt = GBTEngine::load(table, loadfile, false);
	assert(rt == 0);

	double lnglat[4];
	lnglat[0] = 120.2564013;
	lnglat[1] = 30.1798758;
	lnglat[2] = 120.2926970;
	lnglat[3] = 30.2031151;

	rt = GBTEngine::RangeSelect(table, lnglat, outputs);
	assert(rt == 0);
	return rt;

}
