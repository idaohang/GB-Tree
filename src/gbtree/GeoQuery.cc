/*
 * =====================================================================================
 *
 *       Filename:  GeoQuery.cc
 *
 *    Description:  geometry query
 *
 *        Version:  1.0
 *        Created:  04/08/2014 04:22:37 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */
#include <set>
#include <stack>
#include "../storagemanager/GBTFile.h"
#include "../pathmanager/PathManager.h"
#include "../util/Distance.h"
#include "GeoQuery.h"
#include "Geohash.h"
#include "GBTreeNode.h"

static const uint64_t LATITUDE_HOLDER = UINT64_C(0x5555555555555555);
static const uint64_t LONGITUDE_HOLDER = UINT64_C(0xaaaaaaaaaaaaaaaa);


uint32_t GeoQuery::range_precision = 3;
double GeoQuery::default_precision = 0.00521025;
double GeoQuery::default_max_distance = 6371004000.0;

RT GeoQuery::RangeQuery(const char* table, uint64_t left_down, uint64_t right_up, std::vector<RecordId>& outputs)
{
	return RangeQueryImpl(std::string(table), left_down, right_up, outputs);
}

RT GeoQuery::RangeQueryImpl(const std::string &table, uint64_t left_down, uint64_t right_up, 
		 std::vector<RecordId>& outputs)
{
	RT rt;

	bool z_shape = false;
	bool bottom_start = true;
	int tmp_int;// the number of key in leaf node
	int count_node;
	uint64_t key;
	uint64_t max_lat = 0;
	uint64_t max_lng = 0;
	uint64_t min_lat = 0;//used for checking whether the range of page is "Z" type.
	uint64_t tmp_data = 0;
	uint64_t starter = left_down;
	GBTreeIndex gbt_index;
	IndexCursor cursor;
	RecordId rid;
	GBTLeafNode l_node;
	std::stack<uint64_t> min_addresses;
	std::set<PageId> pages;
	std::pair<std::set<PageId>::iterator, bool> ret;

	//check whether the range is valid or not.
	if(! CheckRangeValid(left_down, right_up))
		return RT_GEOQUERY_INVALID_RANGE;

	min_addresses.push(left_down);
	//open the index file
	if((rt = gbt_index.open(PathManager::GetIndexPath(table), 'r')) != 0) return rt;

	//the end 
	while(min_addresses.size() != 0)
	{
		//locate the left-down point in leaf node.
		left_down = min_addresses.top();
		min_addresses.pop();
		if((rt = gbt_index.locate(left_down, cursor)) != 0)
		{
			gbt_index.close();
			return rt;
		}
	
		//judge whether the page has been scanned.
		ret = pages.insert(cursor.pid);
		if(ret.second == false)
			continue;

		//judge whether start the minimal latitude
		if((left_down & LATITUDE_HOLDER) == (starter & LATITUDE_HOLDER))
				bottom_start = true;
		//set the min_lat
		min_lat = left_down & LATITUDE_HOLDER;
		max_lat = 0;
		max_lng = 0;
		//judge whether the smallest address is located in the page.
		tmp_int = cursor.eid;
		cursor.eid = 0;
		gbt_index.loadLeafNode(cursor.pid, l_node); 
		l_node.readEntry(cursor.eid, key, rid);
		if(!((GeoHashCmp(starter, key) == -5) && (GeoHashCmp(right_up, key) == 5)))
			cursor.eid = tmp_int;

		count_node = l_node.getKeyCount();
		//scan the pointer 
		while (cursor.eid != count_node) {
			l_node.readEntry(cursor.eid, key, rid);
				
			if((GeoHashCmp(starter, key) == -5) && (GeoHashCmp(right_up, key) == 5))
			{
				outputs.push_back(rid);
				//get the max latitude.
				tmp_data = key & LATITUDE_HOLDER;
				if(tmp_data > (max_lat & LATITUDE_HOLDER))
					max_lat = key;
				//judge whether the shape of the page is z.
				if(min_lat > tmp_data )
				{
					min_lat = tmp_data;
					z_shape = true;
				}

				tmp_data = key & LONGITUDE_HOLDER;
				if(tmp_data > (max_lng & LONGITUDE_HOLDER))
					max_lng = key;

			}
			cursor.eid++;
		}
		if(max_lng == 0 || max_lat == 0)
			continue;
		//find the right-down point if top page is z shape.
		IndexCursor tmp_cursor;
		tmp_data = (starter & LATITUDE_HOLDER) | (max_lng & LONGITUDE_HOLDER);
		if((rt = gbt_index.locate(tmp_data, tmp_cursor)) != 0){
			gbt_index.close();
			return rt;
		}
		if(tmp_cursor.pid == cursor.pid)
			bottom_start = true;
		//find the right-down point.
		if(bottom_start)
		{
			tmp_data = min_lat | (max_lng & LONGITUDE_HOLDER);
			FindRightDownPoint(tmp_data, key);
			if((tmp_data & LONGITUDE_HOLDER) < (right_up & LONGITUDE_HOLDER))
				min_addresses.push(tmp_data);
			bottom_start = false;
    	
		}
		//find the turn point in z shape page
		if(z_shape)
		{
			FindLeftUpPoint(max_lng, key);
			tmp_data = max_lat;
			FindRightDownPoint(tmp_data, key);
			tmp_data = (max_lng & LATITUDE_HOLDER) | (tmp_data & LONGITUDE_HOLDER);
			if((GeoHashCmp(starter, tmp_data) == -5) && (GeoHashCmp(right_up, tmp_data) == 5))
				min_addresses.push(tmp_data);	
			z_shape = false;
		}
		//find next left-down point.
		tmp_data = (left_down & LONGITUDE_HOLDER) | (max_lat & LATITUDE_HOLDER);
		FindLeftUpPoint(tmp_data, key);
		if((tmp_data & LATITUDE_HOLDER) < (right_up & LATITUDE_HOLDER))
			min_addresses.push(tmp_data);
	}
	gbt_index.close();
	return GEOQUERY_OK;
}
RT GeoQuery::FindLeftUpPoint(uint64_t& left_up, uint64_t max_address )
{
	int i;
	uint64_t latitude = (uint64_t)ExtractLatLng(left_up, 1);
	uint64_t longitude ;
	uint64_t holder = UINT64_C(0x01);
	while(left_up < max_address)
	{
		latitude += (1 << range_precision);	
		longitude = left_up & LONGITUDE_HOLDER;
		for(i = 0; i < 32; i++)
		{
			longitude = longitude | ( (latitude<<i) & (holder << (2*i)) );
		}
		left_up = longitude;
	}
	return 0;
}
RT GeoQuery::FindRightDownPoint(uint64_t& left_down, uint64_t max_address)
{
	int i;
	uint64_t longitude = (uint64_t)ExtractLatLng(left_down, 0);
	uint64_t latitude ;
	uint64_t holder = UINT64_C(0x2);
	while(left_down < max_address)
	{
		longitude += (1 << range_precision);	
		latitude = left_down & LATITUDE_HOLDER;
		for(i = 0; i < 32; i++)
		{
			latitude = latitude | ( (longitude<<(i+1)) & (holder << (2*i)) );
		}
		left_down = latitude;
	}
	return 0;
}
uint32_t GeoQuery::ExtractLatLng(uint64_t address, int type)
{
	uint32_t answer = 0;
	int i;
	if(type)
		for(i = 31; i >= 0; --i)
			answer = (answer<<1) + ((address>>(2*i)) & 0x01);
	else
		for(i = 31; i >= 0; --i)
			answer = (answer<<1) + (address>>(2*i+1) & 0x01);
	
	return answer;
}
bool GeoQuery::CheckRangeValid(uint64_t left_down, uint64_t right_up)
{
	if(GeoHashCmp(left_down, right_up) == -5)
		return true;
	return false;
}
/* *
 * compare two geohash value 
 * @param first[IN] first geohash value
 * @param second[IN] second geohash value
 * @return 0 if first==second, 
 *         -5 if first.longitude < second.longitude And first.latitude < second.latitude
 *         -4 if first.longitude == second.longitude And first.latitude < second.latitude 
 *         -3 if first.longitude > second.longitude And first.latitude < second.latitude
 *         -1 if first.longitude < second.longitude And first.latitude == second.latitude
 *          1 if first.longitude > second.longitude And first.latitude == second.latitude
 *          3 if first.longitude < secon.longitude And first.latitude > second.latitude
 *          4 if first.longitude == second.longitude And first.latitude > second.latitude
 *          5 if first.longitude > second.longitude And first.latitude > second.latitude
 * */
int GeoQuery::GeoHashCmp(uint64_t first, uint64_t second)
{
	if(first == second)
		return 0;
	else{
		uint64_t first_tmp = first & LONGITUDE_HOLDER;
		uint64_t second_tmp = second & LONGITUDE_HOLDER;
		int lng_cmp, lat_cmp;
		if(first_tmp == second_tmp)
			lng_cmp = 0;
		else if(first_tmp < second_tmp)
			lng_cmp = -1;
		else
			lng_cmp = 1;

		first_tmp = first & LATITUDE_HOLDER;
		second_tmp = second & LATITUDE_HOLDER;

		if(first_tmp == second_tmp)
			lat_cmp = 0;
		else if(first_tmp < second_tmp)
			lat_cmp = -4;
		else
			lat_cmp = 4;

		return lng_cmp + lat_cmp;
	}
}

RT GeoQuery::FindPointImpl(GBTreeIndex& gbt_index, uint64_t address, RecordId& outputs)
{
	RT rt;
	IndexCursor cursor;
	uint64_t key;
	RecordId rid;
	uint64_t prec = 1 << range_precision;
	if((rt = gbt_index.locate(address, cursor)) != 0){
		gbt_index.close();
		return rt;
	}	
	if ((rt = gbt_index.readForward(cursor, key, rid)) == 0) {
		if((key - address) < prec)
			outputs = rid;
		else{
			return GEOQUERY_NOT_FOUND;
		}
	}
	return GEOQUERY_OK;

}
RT GeoQuery::FindPoint(const char *table, uint64_t address, RecordId& outputs)
{
	RT rt;
	GBTreeIndex gbt_index;
	if((rt = gbt_index.open(PathManager::GetIndexPath(std::string(table)), 'r')) != 0) return rt;
	rt = FindPointImpl(gbt_index, address, outputs);

	gbt_index.close();
	return rt;
}
RT GeoQuery::Nearest(const char *table, uint64_t address, std::vector<NearestResult>& outputs, size_t count, double min_distance, double max_distance)
{
	RT rt;
	
	int i;
	int bit_start = 0;
	int bit_end = 0;
	double start_precision = default_precision; 
	int count_neighbors = 0;
	int bit_length;
	uint64_t neighbors[8];
	uint64_t key;
	uint64_t holder = 1;
	double lnglat[4] ;
	RecordId rid;
	IndexCursor cursor;
	NearestResult n_result;
	GBTreeIndex gbt_index;
	std::set<NearestResult, NearestResultCmp> answers;


	if(min_distance < default_precision)
		min_distance = default_precision;
	if((max_distance - 0.0) < DOUBLE_EPSILON)
		max_distance = default_max_distance; 
	if((max_distance - min_distance) < DOUBLE_EPSILON)
		return GEOQUERY_INTERNAL_ERROR;

	//get the bit length based on the distance.
	while(((size_t)bit_start < DATA_BIT_PRECISION) && (min_distance - start_precision) > DOUBLE_EPSILON)
	{
		start_precision *= 2;
		bit_start += 2;
		bit_end += 2;
	}
	//get max bit length base on the max_distance.
	if((max_distance - default_max_distance) < DOUBLE_EPSILON)
		bit_end = DATA_BIT_PRECISION;
	else{
		while(((size_t)bit_end < DATA_BIT_PRECISION) && (max_distance - start_precision) > DOUBLE_EPSILON)
		{
			start_precision *= 2;
			bit_end += 2;
		}
	}
	
	if((min_distance - default_precision) > default_precision)
		bit_start -= 2;
	//decode the address
	if((rt = geohash_decode_64(address, lnglat, lnglat+1)) != GEOHASH_OK){
		return rt;
	}
	
	if((rt = gbt_index.open(PathManager::GetIndexPath(std::string(table)), 'r')) != 0) return rt;
	//find the nearest point.
	while(bit_start < bit_end)
	{
		if(answers.size() >= count)
			break;
		bit_length = DATA_BIT_PRECISION - bit_start;
		geohash_neighbors_64(address, bit_length, neighbors, &count_neighbors);
		if((size_t)bit_length == DATA_BIT_PRECISION)
		{
			for(i = 0; i < count_neighbors; i++){
				if(FindPointImpl(gbt_index, neighbors[i], rid) == GEOQUERY_OK){
					if((rt = geohash_decode_64(neighbors[i], lnglat+2, lnglat+3)) != GEOHASH_OK){
						gbt_index.close();
						return rt;
					}
					n_result.rid = rid;
					n_result.distance = LatLon2Dist(lnglat[0], lnglat[1], lnglat[2], lnglat[3]);
					answers.insert(n_result);
				}
			}
		}
		else
		{
			for(i = 0; i < count_neighbors; i++)
			{
				if((rt = gbt_index.locate(neighbors[i], cursor)) != 0){
					gbt_index.close();
					return rt;
				}
				neighbors[i] += (holder<<bit_start);
				if((rt = gbt_index.readForward(cursor, key, rid)) != 0){
					gbt_index.close();
					return rt;
				}
				while(key < neighbors[i]){
					if((rt = geohash_decode_64(key, lnglat+2, lnglat+3)) != GEOHASH_OK){
						gbt_index.close();
						return rt;
					}
					n_result.rid = rid;
					n_result.distance = LatLon2Dist(lnglat[0], lnglat[1], lnglat[2], lnglat[3]);
					answers.insert(n_result);
					if((rt = gbt_index.readForward(cursor, key, rid)) != 0){
						gbt_index.close();
						return rt;
					}
				}
			}
			
		}
		bit_start += 2;
	}
	gbt_index.close();

	i = 0;
	std::set<NearestResult, NearestResultCmp>::iterator it;
	for(it = answers.begin(); it != answers.end() && ((size_t)i < count); ++it, ++i)
		outputs.push_back(*it);

	return GEOQUERY_OK;

}
