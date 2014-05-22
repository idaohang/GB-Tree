/*
 * =====================================================================================
 *
 *       Filename:  ProgException.cc
 *
 *    Description:  program exception class
 *
 *        Version:  1.0
 *        Created:  05/20/2014 10:31:18 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include "ProgException.h"


IndexOutOfBoundsException::IndexOutOfBoundsException(int code, const char* str)
{
	error_code = code;
	error_string = string(str);
}
std::string IndexOutOfBoundsException::What() const
{
	return string(itoa(error_code)) + ": " + error_string;
}

IllegalStateException::IllegalStateException(int code, const char* str)
{
	error_code = code;
	error_string = string(str);
}
std::string IllegalStateException::What() const
{
	return string(itoa(error_code)) + ": " + error_string;
}
