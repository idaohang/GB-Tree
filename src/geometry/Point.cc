/*
 * =====================================================================================
 *
 *       Filename:  Point.cc
 *
 *    Description:  the class of point Geometry
 *
 *        Version:  1.0
 *        Created:  05/19/2014 03:51:09 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include <string.h>
#include <math.h>
#include <limits>

#include "Point.h"

Point::Point()
	:dimension(0), coordinates(0)
{
}

Point::Point(const double* coordinates, uint32_t dimension) 
{
	this->coordinates = new double[dimension];
	memcpy(this->coordinates, coordinates, dimension*sizeof(double));
	this->dimension = dimension;
}
Point::Point(const Point& p)
{
	this->dimension = p.GetDimesion();
	this->coordinates = new double[dimension];
	p.GetCoordinates(this->coordinates);
}

Point& Point::operator=(const Point& p)
{
	if(this != p)
	{
		SetDimension(p.GetDimesion());
		memcpy(this->coordinates, coordinates, dimension*sizeof(double));
	}
	return *this;
}

bool Point::operator==(const Point& p)const
{
	if(dimension != p.GetDimesion())
		return false;
	uint32_t i;
	for(i = 0; i < dimension; ++i)
	{
		if(p.GetCoordinate(i) > coordinates[i] + std::numeric_limits<double>::epsilon() || 
				p.GetDimesion(i) < coordinates[i] - std::numeric_limits<double>::epsilon())
			return false;
	}
	return true;
}

//ISerializable Interfaces
uint32_t Point::GetByteArraySize()
{
	return (sizeof(uint32_t) + dimension*sizeof(double));
}

void Point::LoadFromByteArray(const byte* data)
{
	uint32_t d
	memcpy(&d, data, sizeof(uint32_t));
	SetDimension(d);
	data += sizeof(uint32_t);
	memcpy(coordinates, data, dimension*sizeof(double));
}
void Point::StoreToByteArray(byte** data, uint32_t& length)
{
	length = GetByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;
	memcpy(ptr, &dimension, sizeof(uint32_t));

	ptr += sizeof(uint32_t);
	memcpy(ptr, coordinates, dimension*sizeof(double));
	
}
//IGeometry Interface
bool Point::IntersectGeometry(const IGeometry& input) const
{

}
bool Point::ContainGeometry(const IGeometry& input)const
{
	return false;
}
bool Point::TouchGeometry(const IGeometry& input) const
{

}
void Point::GetCenter(Point& out) const
{
	out = *this;
}
uint32_t Point::GetMBR(Region& out) const
{

}
double Point::GetArea() const
{
	return 0.0;
}
double Point::GetMinimumDistance(const IGeometry& input) const
{

}
double Point::GetMinimumDistance(const Point& p) const
{

}
double Point::GetCoordinate(uint32_t index)const
{
	return coordinates[index];
}

double* Point::GetCoordinates() const
{
	return coordinates;
}
void Point::SetDimension(const uint32_t d)
{
	if(dimension != d){
		delete []coordinates;
		coordinates = 0;
		dimension = d;
		coordinates = new double[dimension];
	}
}
