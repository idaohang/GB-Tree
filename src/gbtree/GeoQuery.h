/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */

#ifndef GEOQUERY_H_
#define GEOQUERY_H_

#include <vector>
#include <string>
#include "../base/GBTreeBase.h"
#include "GBTTable.h"
#include "GBTreeNode.h"
#include "GBTreeIndex.h"

enum{
	GEOQUERY_OK,
	GEOQUERY_NOT_FOUND,
	GEOQUERY_INTERNAL_ERROR
};
typedef struct _NearestResult{
	RecordId rid;
	double distance;
}NearestResult;
//compare function used for set
typedef struct _NearestResultCmp{
	bool operator()(const NearestResult& n1, const NearestResult& n2) const
	{
		return n1.distance < n2.distance;
	}
}NearestResultCmp;
class GeoQuery
{
	public:
		/* *
		 * the precision used for finding next left-down point in range query
		 * */
		static uint32_t range_precision;
		/* *
		 * find point based on the address
		 * */
		static RT FindPoint(const char *table, uint64_t adress, RecordId& outputs);
		/* *
		 * range query 
		 * @param table[IN] the name of table
		 * @param left_down[IN] left-down point of range.
		 * @param right_up[IN] right-down point of range.
		 * @param outputs[OUT] the answers of query.
		 * @return 0 if succeed.
		 * */
		static RT RangeQuery(const char* table, uint64_t left_down, uint64_t right_up, std::vector<RecordId>& outputs);

		/* *
		 * find n Nearest points arount the point
		 * @param table[IN] the table name
		 * @param address[IN] the address of the point
		 * @param count[IN] the number of the answer
		 * @param min_distance[IN] minimal distance between the address and answer
		 * */
		static RT Nearest(const char *table, uint64_t address, std::vector<NearestResult>& outputs, size_t count=50, double min_distance=default_precision, double max_distance=default_max_distance);

	private:

		/* *
		 * check whether the range is invalid or not.
		 * @param left_down[IN] leaf-down point of range.
		 * @param right_up[IN] right-down point of range.
		 * @return true if succeed.
		 * */
		static bool CheckRangeValid(uint64_t left_down, uint64_t right_up); 

		/* *
		 * compare two geohash value 
		 * @param first[IN] first geohash value
		 * @param second[IN] second geohash value
		 * @return 0 if first==second, -1 if first < second, 1 if first > second.
		 * */
		static int GeoHashCmp(uint64_t first, uint64_t second);

		/* *
		 * Range Query implementation.
		 * */
		static RT RangeQueryImpl(const std::string& table, uint64_t left_down, uint64_t right_up, 
				 std::vector<RecordId>& outputs);

		/* *
		 * find the right up address(namely next leaf-down address).
		 * */
		static RT FindLeftUpPoint(uint64_t& left_up, uint64_t max_address );

		/* *
		 * find the left-down address
		 * */
		static RT FindRightDownPoint(uint64_t& left_down, uint64_t max_address);

		/* *
		 * extract the latitude and longitude.
		 * @param geo hash address
		 * @param 0 for longitude, 1 for latitude
		 * @return longitude or latitude
		 * */
		static uint32_t ExtractLatLng(uint64_t address, int type);
	private:
		static double default_precision;
		static double default_max_distance;
		static RT FindPointImpl(GBTreeIndex& gbt_index, uint64_t address, RecordId& outputs); 
};
#endif
