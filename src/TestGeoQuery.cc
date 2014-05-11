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
#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "TestGeoQuery.h"
#include "GBTEngine.h"
#include "PathManager.h"
#include "GBTFile.h"


static void GetData(std::string table, double *coordinates)
{

	if(table.compare("small") == 0){
		coordinates[0] = 120.9333383;
		coordinates[1] = 30.827487;
		coordinates[2] = 120.6656566;
		coordinates[3] = 30.9074838;
		coordinates[4] = 120.9515271;
		coordinates[5] = 30.84805509; 
	}
	else if(table.compare("middle") == 0){
		coordinates[0] = 120.9333383;
		coordinates[1] = 30.827487;
		coordinates[2] = 120.2657404;
		coordinates[3] = 30.30048649;
		coordinates[4] = 120.456645;
		coordinates[5] = 30.91548937; 
	}
	else if(table.compare("large") == 0){
		coordinates[0] = 120.7402979;
		coordinates[1] = 36.9743701;
		coordinates[2] = 120.3607551;
		coordinates[3] = 36.1212045;
		coordinates[4] = 120.4015953;
		coordinates[5] = 36.20051595; 
	}
	else if(table.compare("xlarge") == 0){
		coordinates[0] = 116.2369216;
		coordinates[1] = 39.99827827;
		coordinates[2] = 116.6549728;
		coordinates[3] = 39.88331447;
		coordinates[4] = 116.3136523;
		coordinates[5] = 39.97118243; 
	}
	else if(table.compare("xxlarge") == 0){
		coordinates[0] = 113.2778673;
		coordinates[1] = 23.8197065;
		coordinates[2] = 113.4583184;
		coordinates[3] = 23.88698923;
		coordinates[4] = 116.3136523;
		coordinates[5] = 39.97118243; 
	}else{
		coordinates[0] = 113.2778673;
		coordinates[1] = 23.8197065;
		coordinates[2] = 120.2892393;
		coordinates[3] = 30.83898665;
		coordinates[4] = 116.3136523;
		coordinates[5] = 39.97118243; 
	}
}

int TestPointSelect(const char* table_name, const char*data_file)
{
	std::string value;
	std::string table(table_name);
	std::string loadfile(data_file);
	GBTEngine::load(table, loadfile, true);
	double coordinates[6];
	GetData(table, coordinates);
	int count = 10000;
	int i;
	struct timeval start, stop;
	int     bpagecnt, epagecnt;
	
	bpagecnt = GBTFile::getPageReadCount();
	gettimeofday(&start, NULL);
	for(i = 0; i < count; i++)
	{
		GBTEngine::EqualSelect(table, coordinates[0], coordinates[1], value);
		GBTEngine::EqualSelect(table, coordinates[2], coordinates[3], value);
		GBTEngine::EqualSelect(table, coordinates[4], coordinates[5], value);
	}
	gettimeofday(&stop, NULL);
	epagecnt = GBTFile::getPageReadCount();
	count *= 3;
	double duration = (double)(stop.tv_usec - start.tv_usec) / 1000000.0;
	fprintf(stdout, "-- the duration is %.5f, the qps is %.5f, read %d pages\n", duration,  ((float)count) / duration, epagecnt - bpagecnt);
	return 0;
}
static void GetRange(std::string name, double *lnglat)
{
		lnglat[0] = 120.2564013;
		lnglat[1] = 30.1798758;
		lnglat[2] = 120.2926970;
		lnglat[3] = 30.2031151;

}
int TestRangeQuery(const char* table_name, const char*data_file)
{
	int rt;
	std::string value;
	std::string table(table_name);
	std::string loadfile(data_file);
	std::vector<std::string> outputs;
	rt = GBTEngine::load(table, loadfile, false);
	assert(rt == 0);

	double lnglat[4];
	GetRange(table, lnglat);
	struct timeval start, stop;
	int     bpagecnt, epagecnt;
	bpagecnt = GBTFile::getPageReadCount();

	gettimeofday(&start, NULL);
	rt = GBTEngine::RangeSelect(table, lnglat, outputs);
	gettimeofday(&stop, NULL);
	epagecnt = GBTFile::getPageReadCount();
	fprintf(stdout, "--Page size is %d,  -- %lu microseconds to run the range command. Read %d pages\n",
			GBTFile::PAGE_SIZE, stop.tv_usec - start.tv_usec, epagecnt - bpagecnt);
	assert(rt == 0);
	return rt;
}
static void GetNearestCenter(std::string table, double *coordinates)
{

	if(table.compare("small") == 0){
		coordinates[0] = 120.6656566;
		coordinates[1] = 30.9074838;
	}
	else if(table.compare("middle") == 0){
		coordinates[0] = 120.2657404;
		coordinates[1] = 30.30048649;
	}
	else if(table.compare("large") == 0){
		coordinates[0] = 120.3607551;
		coordinates[1] = 36.1212045;
	}
	else if(table.compare("xlarge") == 0){
		coordinates[0] = 116.6549728;
		coordinates[1] = 39.88331447;
	}
	else if(table.compare("xxlarge") == 0){
		coordinates[0] = 113.4583184;
		coordinates[1] = 23.88698923;
	}else{
		coordinates[0] = 120.2892393;
		coordinates[1] = 30.83898665;
	}
}
int TestNearestQuery(const char* table_name, const char*data_file)
{
	int rt;
	size_t count;
	std::string value;
	std::string table(table_name);
	std::string loadfile(data_file);
	std::vector<NearResult_t> outputs;
	rt = GBTEngine::load(table, loadfile, false);
	assert(rt == 0);

	double lnglat[2];
	GetNearestCenter(table, lnglat);
	double min_distance = 0;
	double max_distance = 0;
	count = 2000;
	struct timeval start, stop;
	int     bpagecnt, epagecnt;
	bpagecnt = GBTFile::getPageReadCount();

	gettimeofday(&start, NULL);
	rt = GBTEngine::NearestSelect(table, lnglat, outputs, count, min_distance, max_distance);
	gettimeofday(&stop, NULL);
	epagecnt = GBTFile::getPageReadCount();
	fprintf(stdout, "--Page size is %d,  -- %lu microseconds to run the range command. Read %d pages\n",
			GBTFile::PAGE_SIZE, stop.tv_usec - start.tv_usec, epagecnt - bpagecnt);
	assert(outputs.size() == count);

	return rt;
}
