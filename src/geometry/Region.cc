/*
 * =====================================================================================
 *
 *       Filename:  Region.cc
 *
 *    Description:  region class
 *
 *        Version:  1.0
 *        Created:  05/20/2014 09:28:34 AM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:   (Qi Liu), liuqi.edward@gmail.com
 *   Organization:  antq.com
 *
 * =====================================================================================
 */

#include <string.h>
#include <cmath>
#include <limits>
#include "Region.h"

Region::Region()
	:dimension(0), low(0), high(0)
{
}

Region::Region(const double* low, const double* high, uint32_t dimension)
{
	initialize(low, high, dimension);
}
Region::Region(const Region& r)
{
	initialize(r.GetLowArray(), r.GetHighArray(), r.GetDimesion());
}
Region::~Region()
{
	delete[] low;
	delete[] high;
	low = high = 0;
}

void Region::initialize(const double* low, const double* high, uint32_t dimension)
{
	this->dimension = dimension;
	try
	{
		this->low = new double[dimension];
		this->high = new double[dimension];
	}
	catch(...)
	{
		delete[] this->low;
		throw;
	}
	memcpy(this->low, low, dimension*sizeof(double));
	memcpy(this->high, high, dimension*sizeof(double));
}
Region& Region::operator=(const Region& r)
{
	if(this != &r)
	{
		SetDimension(r.GetDimesion());
		memcpy(low, r.GetLowArray(), dimension*sizeof(double));
		memcpy(high, r.GetHighArray(), dimension*sizeof(double));
	}
	return *this;
}
const bool Region::operator==(const Region& r)const
{
	if(dimension != r.GetDimesion())
		return false;
	double *plow = r.GetLowArray();
	double *phigh = r.GetHighArray();
	int i;
	for(i = 0; i < dimension; ++i)
	{
		if(plow[i] < low[i]-std::numeric_limits<double>::epsilon() ||
				plow[i] > low[i]+std::numeric_limits<double>::epsilon() ||
				phigh[i] < high[i]-std::numeric_limits<double>::epsilon() ||
				phigh[i] > high[i]+std::numeric_limits<double>::epsilon())
			return false;
	}
	return true;
}
//ISerializable Interfaces
uint32_t Region::GetByteArraySize()
{
	return (sizeof(uint32_t) + 2*dimension*sizeof(double));
}
void Region::LoadFromByteArray(const byte* data)
{
	uint32_t tmp_dimension;
	memcpy(&tmp_dimension, data, sizeof(uint32_t));

	if(tmp_dimension != dimension)
		SetDimension(tmp_dimension);
	data += sizeof(uint32_t);
	memcpy(low, data, dimension*sizeof(double));
	data += dimension*sizeof(double);
	memcpy(high, data, dimension*sizeof(double));
}
void Region::StoreToByteArray(byte** data, uint32_t& length)
{
	length = GetByteArraySize();
	*data = new byte[length];
	byte* ptr = *data;

	memcpy(ptr, &dimension, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, low, dimension*sizeof(double));
	ptr += dimension*sizeof(double);
	memcpy(ptr, high, dimension*sizeof(double));
}
//IShape Interfaces
bool Region::IntersectGeometry(const IGeometry& input)const
{
	const Region* pr = dynamic_cast<const Region*>(&input);
	if(pr != NULL) 
		return IntersectRegion(*pr);
	const Point* pp = dynamic_cast<const Point*>(&input);
	if(pp != NULL)
		return ContainPoint(*pp);

	return false;
}
bool Region::ContainGeometry(const IGeometry& input)const
{
	const Region* pr = dynamic_cast<const Region*>(&input);
	if(pr != NULL)
		return ContainRegion(*pr);
	const Point* pp = dynamic_cast<const Point*>(&input);
	if(pp != NULL)
		return ContainPoint(*pp);

	return false;
}
bool Region::TouchGeometry(const IGeometry& input)const
{
	const Region* pr = dynamic_cast<const Region*>(&input);
	if(pr != NULL)
		return TouchRegion(*pr);
	const Point* pp = dynamic_cast<const Point*>(&input);
	if(pp != NULL)
		return TouchPoint(*pp);

	return false;
}
void Region::GetCenter(Point& out) const
{
	out.SetDimension(dimension);
	const double* pcoord = out.GetCoordinates();
	int i;
	for(i = 0; i < dimension; ++i)
		pcoord[i] = (low[i] + high[i]) / 2;
}
uint32_t Region::GetDimesion() const
{
	return dimension;
}
void Region::GetMBR(Region& out) const
{
	out = *this;
}
double Region::GetArea()const
{
	double area = 1.0;
	int i;
	for(i = 0; i < dimension; ++i)
		area *= (high[i] - low[i]);

	return area;
}
double Region::GetMinimumDistance(const IGeometry& in) const
{
	const Region* pr = dynamic_cast<const Region*>(&in);
	if(pr != NULL)
		return GetMinimumDistance(*pr);
	const Point* pp = dynamic_cast<const Point*>(&in);
	if(pp != NULL)
		return GetMinimumDistance(*pp);

	throw Indexes::IllegalStateException(ERROR_NOT_IMPLETEMENTED, 
			"Minimum Distance not implemented yet!");
	
}
bool Region::IntersectRegion(const Region& input)const
{
	if(dimension != input.GetDimesion())
		return false;
	const double* plow = input.GetLowArray();
	const double* phigh = input.GetHighArray();
	int i;
	for(i = 0; i < dimension; ++i)
	{
		if(low[i] > phigh[i] || high[i] < plow[i])
			return false;
	}
	return true;
}
bool Region::ContainRegion(const Region& input)const
{
	if(dimension != input.GetDimesion())
		return false;
	const double* plow = input.GetLowArray();
	const double* phigh = input.GetHighArray();
	int i;
	for(i = 0; i < dimension; ++i)
	{
		if(low[i] > plow[i] || high[i] < phigh[i])
			return false;
	}
	return true;
}
bool Region::TouchRegion(const Region& input)const
{
	if(dimension != input.GetDimesion())
		return false;
	const double* plow = input.GetLowArray();
	const double* phigh = input.GetHighArray();
	int i;
	for(i = 0; i < dimension; ++i)
	{
		if(((low[i] >= plow[i] + std::numeric_limits<double>::epsilon())&&
				(low[i] <= plow[i] - std::numeric_limits<double>::epsilon()))||
				((high[i] >= phigh[i] + std::numeric_limits<double>::epsilon()) &&
				 (high[i] <= phigh[i] - std::numeric_limits<double>::epsilon())))
			return false;
	}
	return true;
}
double Region::GetMinimumDistance(const Region& input)const
{
	if(dimension != input.GetDimesion())
		throw Indexes::IllegalStateException(ERROR_DIFFERENT_DIMENSION,
				"Region::GetMinimumDistance: Regions have different number of dimension");
	const double* plow = input.GetLowArray();
	const double* phigh = input.GetHighArray();
	double ret = 0.0;
	uint32_t i;
	for(i = 0; i < dimension; ++i)
	{
		double x = 0.0;
		if(high[i] < plow[i])
			x = std::abs(plow[i] - high[i]);
		else if(phigh[i] < low[i])
			x = std::abs(low[i] - phigh[i]);
		ret += x * x;
	}
	return sqrt(ret);
}
bool Region::ContainPoint(const Point& input)const
{
	if(dimension != input.GetDimesion())
		return false;
	const double* pc = input.GetCoordinates();
	int i;
	for(i = 0; i < dimension; ++i)
	{
		if(low[i] > pc[i] || high[i] < pc[i])
			return false;
	}
	return true;
}
bool Region::TouchPoint(const Point& input)const
{
	if(dimension != input.GetDimesion())
		return false;
	const double* pc = input.GetCoordinates();
	int i;
	for(i = 0; i < dimension; ++i)
	{
		if(((low[i] >= plow[i] - std::numeric_limits<double>::epsilon())&&
				(low[i] <= plow[i] + std::numeric_limits<double>::epsilon())) ||
				((high[i] >= phigh[i] - std::numeric_limits<double>::epsilon()) &&
				 (high[i] <= phigh[i] + std::numeric_limits<double>::epsilon())))
			return true;
	}
	return false;
}
double Region::GetMinimumDistance(const Point& input)const
{
	if(dimension != input.GetDimesion())
		throw Indexes::IllegalStateException(ERROR_DIFFERENT_DIMENSION,
				"Region::GetMinimumDistance: Regions have different number of dimension");
	const double* pp = input.GetCoordinates();
	double ret = 0.0;
	uint32_t i;
	for(i = 0; i < dimension; ++i)
	{
		double x = 0.0;
		if(high[i] < pp[i])
			x = pp[i] - high[i];
		else if(pp[i] < low[i])
			x = low[i] - pp[i];
		ret += x * x;
	}
	return sqrt(ret);
}
double Region::GetLow(uint32_t index) const
{
	if(index >= dimension)
		throw Indexes::IndexOutOfBoundsException(OUT_OF_BOUNDRY, "Region: beyond the dimension");

	return low[index];

}
double Region::GetHigh(uint32_t index) const
{
	if(index >= dimension)
		throw Indexes::IndexOutOfBoundsException(OUT_OF_BOUNDRY, "Region: beyond the dimension");

	return high[index];
}
double* Region::GetLowArray(double *low, uint32_t count) const{
	return low;
}
double* Region::GetHighArray()const
{
	return high;
}
uint32_t Region::GetDimesion() const
{
	return dimension;
}
void Region::SetDimension(const uint32_t dimension)
{
	if(this->dimension != dimension)
	{
		this->dimension = dimension;
		delete[] low;
		delete[] high;
		low = 0;
		high = 0;
		low = new double[dimension];
		high = new double[dimension];
	}
}
