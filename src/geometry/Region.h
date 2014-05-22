/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
#ifndef GEOMETRY_REGION_H_
#define GEOMETRY_REGION_H_
#include "GeometryBase.h"
#include "Point.h"

namespace Geometry{
	class Region : public virtual IGeometry{
		public:
			Region();
			Region(const double* low, const double* high, uint32_t dimension);
			Region(const Region& p);
			virtual ~Region();

			virtual Region& operator=(const Region& p);
			virtual bool operator==(const Region& p) const;

			//ISerializable interface
			virtual uint32_t GetByteArraySize();
			virtual void LoadFromByteArray(const byte* data);
			virtual void StoreToByteArray(byte** data, uint32_t& length);

			//IShape interface
			virtual bool IntersectGeometry(const IGeometry& input)const;
			virtual bool ContainGeometry(const IGeometry& input) const;
			virtual bool TouchGeometry(const IGeometry& input) const;

			virtual void GetCenter(Point& out) const;
			virtual uint32_t GetDimesion() const;
			virtual void GetMBR(Region& out) const;
			virtual double GetArea() const;
			virtual double GetMinimumDistance(const IGeometry& in) const;

			//Region numbers
			virtual bool IntersectRegion(const Region& input) const;
			virtual bool ContainRegion(const Region& input) const;
			virtual bool TouchRegion(const Region& input) const;
			virtual double GetMinimumDistance(const Region& input)const;

			virtual bool ContainPoint(const Point& input)const;
			virtual bool TouchPoint(const Point& input) const;
			virtual double GetMinimumDistance(const Point& input)const;

			virtual double GetLow(uint32_t index) const;
			virtual double GetHigh(uint32_t index) const;
			virtual double* GetLowArray() const;
			virtual double* GetHighArray()const;
			virtual uint32_t GetDimesion() const;


		private:
			uint32_t dimension;
			double* low;
			double* high;

		private:
			void SetDimension(const uint32_t d);
			void initialize(const double* low, const double* high, uint32_t dimension);

	};
}
#endif
