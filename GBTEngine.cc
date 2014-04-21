/*
 * =====================================================================================
 *
 *       Filename:  GBTEngine.cc
 *
 *    Description:  load data and select data
 *
 *        Version:  1.0
 *        Created:  04/02/2014 06:25:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */
#define __STDC_FORMAT_MACROS

#include <sys/times.h>
#include <stdlib.h>
#include <inttypes.h>
#include "GBTFile.h"
#include "GBTEngine.h"
#include "GBTTable.h"
#include "GBTreeIndex.h"
#include "Geohash.h"
#include "PathManager.h"
#include "GeoQuery.h"
RT GBTEngine::load(const std::string& table, const std::string& loadfile, bool index)
{
	RT ans;
	
	FILE *data_file = fopen(loadfile.c_str(), "r");
	if(data_file == NULL){
		fprintf(stderr, "open data file error!\n");
		return RT_FILE_OPEN_FAILED;
	}

	GBTTable table_file;
	if((ans = table_file.open("data/" + table+".tbl", 'w')) < 0){
		fprintf(stderr, "open table file error!\n");
		fclose(data_file);
		table_file.close(); 
		return RT_FILE_OPEN_FAILED;
	}

	GBTreeIndex index_file(index);
	if((ans = index_file.open("data/"+table+".idx", 'w')) < 0){
		fprintf(stderr, "open index file error!\n");
		fclose(data_file);
		table_file.close();
		index_file.close();
		return RT_FILE_OPEN_FAILED;
	}
	
	uint64_t key;
	char data_string[1024];
	double lng, lat;
	std::string value;
	RecordId rcid;
	int count = 0;
	while(fgets(data_string, 1024, data_file) != NULL){
		if(parseLoadLine(data_string, lng, lat, value) == 0){
			key = 0;
			if(geohash_encode_64(lat, lng, &key) != GEOHASH_OK)
				return RT_GEOHASH_ERROR;

			if((ans = table_file.append(key, value, rcid)) < 0){
				fprintf(stderr, "insert the data into table failed!");
				fclose(data_file);
				table_file.close();
				index_file.close();
				return ans;
			}

			if((ans = index_file.insert(key, rcid)) < 0){
				fprintf(stderr, "Error ID: %d,insert the data into index file failed!", ans);
				fclose(data_file);
				table_file.close();
				index_file.close();
				return ans;
			}
			count++;
		}
	}

	fclose(data_file);
	table_file.close();
	index_file.close();

#ifdef DEBUG
	// debug : print tree
	IndexCursor cursor;
	GBTreeIndex dbt;
	dbt.open("data/" + table + ".idx", 'r');
	
	printf("Tree height: %d\n", dbt.getTreeHeight());
	dbt.pointToSmallestKey(cursor);

	RecordId rid;
	int total = 0;
	key = 0;
	while (dbt.readForward(cursor, key, rid) == 0){
		printf("%d\n", cursor.pid);
		printf("{ key: %" PRIx64 "", key);
		printf(", pid: %d | %d } \n", rid.pid, rid.sid) ;
		++total;
	};

	printf("total: %d\n", total);
	dbt.close();
#endif

	return 0;
}
RT GBTEngine::parseLoadLine(char* line, double& lng, double& lat, std::string& value)
{
	const char* delimiter = ",\r\n";

	char *pch = NULL;
	pch = strtok(line, delimiter);
	lng = atof(pch);
	pch = strtok(NULL, delimiter);
	lat = atof(pch);
	pch = strtok(NULL, delimiter);
	value.assign(pch);
	return 0;
}

RT GBTEngine::EqualSelectImpl(const std::string table, double longitude, double latitude, std::string& value)
{
	RT rt;
	GBTTable table_file;
	RecordId rid;
	uint64_t key = 0;
	if(geohash_encode_64(latitude, longitude, &key) != GEOHASH_OK)
		return RT_GEOHASH_ERROR;
	if(GeoQuery::FindPoint(table.c_str(), key, rid) == GEOQUERY_OK)
	{
		if((rt = table_file.open("data/"+table + ".tbl", 'r')) < 0)
			return rt;

		if((rt = table_file.read(rid, key, value)) != 0)
		{
			table_file.close();
			return rt;
		}
		
		fprintf(stdout, "%f, %f, %s\n", longitude, latitude, value.c_str()); 
	}
	table_file.close();
	return 0;

}
RT GBTEngine::EqualSelect(const std::string& table, double longitude, double latitude, std::string& value)
{
	RT rt;
	struct tms tmsbuf;
	clock_t btime, etime;
	int     bpagecnt, epagecnt;
	
	btime = times(&tmsbuf); 
	bpagecnt = GBTFile::getPageReadCount();
	rt = EqualSelectImpl(table, longitude, latitude, value);
	etime = times(&tmsbuf);
	epagecnt = GBTFile::getPageReadCount();
	fprintf(stderr, "  -- %.5f seconds to run the select command. Read %d pages\n", ((float)(etime - btime))/sysconf(_SC_CLK_TCK), epagecnt - bpagecnt);
	return rt;

}

RT GBTEngine::RangeSelectImpl(const std::string table, double* lnglat, std::vector<std::string>& values)
{
	RT rt;
	std::vector<RecordId> outputs;
	uint64_t starter = 0, end = 0;
	uint64_t key = 0;
	std::string value;
	GBTTable table_file;
	if(geohash_encode_64(lnglat[1], lnglat[0], &starter) != GEOHASH_OK)
		return RT_GEOHASH_ERROR;
	
	if(geohash_encode_64(lnglat[3], lnglat[2], &end) != GEOHASH_OK)
		return RT_GEOHASH_ERROR;
	if(GeoQuery::RangeQuery(table.c_str(), starter, end, outputs) == GEOQUERY_OK)
	{
		if(outputs.size() == 0)
			return 0;

		if((rt = table_file.open(PathManager::GetTablePath(table), 'r')) < 0)
			return rt;
		std::vector<RecordId>::iterator it;
		for(it = outputs.begin(); it != outputs.end(); ++it)
		{
			if((rt = table_file.read(*it, key, value)) != 0)
			{
				table_file.close();
				return rt;
			}
			values.push_back(value);
			fprintf(stdout, "%s\n", value.c_str());
		}
	}
	table_file.close();
	return 0;

}
RT GBTEngine::RangeSelect(const std::string table, double* lnglat, std::vector<std::string>& values)
{
	RT rt;
	struct tms tmsbuf;
	clock_t btime, etime;
	int     bpagecnt, epagecnt;
	btime = times(&tmsbuf); 
	bpagecnt = GBTFile::getPageReadCount();
	rt = RangeSelectImpl(table, lnglat, values);
	etime = times(&tmsbuf);
	epagecnt = GBTFile::getPageReadCount();
	fprintf(stderr, "  -- %.5f seconds to run the select command. Read %d pages\n", ((float)(etime - btime))/sysconf(_SC_CLK_TCK), epagecnt - bpagecnt);
	return rt;
}
RT GBTEngine::NearestSelectImpl(const std::string table, double* lnglat, std::vector<NearResult_t>& outputs, size_t count, double min_distance, double max_distance )
{
	RT rt;
	std::vector<NearestResult> nearests;
	uint64_t key = 0;
	double longitude, latitude;
	std::string value;
	GBTTable table_file;
	NearResult_t result;
	if(geohash_encode_64(lnglat[1], lnglat[0], &key) != GEOHASH_OK)
		return RT_GEOHASH_ERROR;

	if(GeoQuery::Nearest(table.c_str(), key, nearests, count, min_distance, max_distance) == GEOQUERY_OK)
	{
		if(nearests.size() == 0)
			return 0;

		if((rt = table_file.open(PathManager::GetTablePath(table), 'r')) < 0)
			return rt;
		std::vector<NearestResult>::iterator it;
		for(it = nearests.begin(); it != nearests.end(); ++it)
		{
			if((rt = table_file.read(it->rid, key, value)) != 0)
			{
				table_file.close();
				return rt;
			}
			if(geohash_decode_64(key, &latitude, &longitude) == GEOHASH_OK)
			{
				result.longitude = longitude;
				result.latitude = latitude;
				result.value = value;
				result.distance = it->distance;
			}
			outputs.push_back(result);
			fprintf(stdout, "%s, dis: %.5f\n", value.c_str(), it->distance);
		}
	}
	table_file.close();
	return 0;

}
RT GBTEngine::NearestSelect(const std::string table, double* lnglat, std::vector<NearResult_t>& outputs, size_t count, double min_distance, double max_distance )
{
	RT rt;
	struct tms tmsbuf;
	clock_t btime, etime;
	int     bpagecnt, epagecnt;
	btime = times(&tmsbuf); 
	bpagecnt = GBTFile::getPageReadCount();
	rt = NearestSelectImpl(table, lnglat, outputs, count, min_distance, max_distance);
	etime = times(&tmsbuf);
	epagecnt = GBTFile::getPageReadCount();
	fprintf(stderr, "  -- %.5f seconds to run the select command. Read %d pages\n", ((float)(etime - btime))/sysconf(_SC_CLK_TCK), epagecnt - bpagecnt);
	return rt;

}
