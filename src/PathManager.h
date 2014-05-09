/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */

#ifndef PATH_MANAGER_H_
#define PATH_MANAGER_H_

#include <string>

class PathManager{

public:
	/* *
	 * the data directory
	 * */
	static std::string data_directory;
	/* *
	 * get the index file path
	 * */
	static std::string GetIndexPath(std::string table);

	static std::string GetTablePath(std::string table);
private:
	static const char* index_extension;
	static const char* table_extension;
};

#endif
