/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
#ifndef PROGRAM_EXCEPTION_H_
#define PROGRAM_EXCEPTION_H_
#include <string>
#include "Tools.h"


namespace Indexes
{
	const int NEW_MEM_ERROR = -10001;
	const int OUT_OF_BOUNDRY = -11001;
	const int ERROR_NOT_IMPLETEMENTED = -12001;
	const int ERROR_DIFFERENT_DIMENSION = -12002;
	class Exception
	{
		public:
			virtual std::string What() const {}
			virtual ~Exception() {};
	};

	class IndexOutOfBoundsException: public Exception
	{
		public:
			IndexOutOfBoundsException(int code, const char* str);
			virtual ~IndexOutOfBoundsException() {}
			virtual std::string What() const;
		private:
			int error_code;
			std::string error_string;
	};
	class IllegalStateException: public Exception
	{
		public:
			IllegalStateException(int code, const char* str);
			virtual ~IllegalStateException() {}
			virtual std::string What() const;
		private:
			int error_code;
			std::string error_string;
	}
	
}

#endif
