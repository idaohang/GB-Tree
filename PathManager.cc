/*
 * =====================================================================================
 *
 *       Filename:  PathManager.cc
 *
 *    Description:  path manager
 *
 *        Version:  1.0
 *        Created:  04/18/2014 11:11:26 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include "PathManager.h"

std::string PathManager::data_directory = "data/";
const char* PathManager::index_extension = ".idx";
const char* PathManager::table_extension = ".tbl";

std::string PathManager::GetIndexPath(std::string table)
{
	return data_directory + table + index_extension;
}

std::string PathManager::GetTablePath(std::string table)
{
	return data_directory + table + table_extension;
}
