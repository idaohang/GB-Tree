/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
#ifndef GEOMETRY_POINT_H_
#define GEOMETRY_POINT_H_
#include "GeometryBase.h"

namespace Geometry{
	class Point : public virtual IGeometry{
		public:
			Point();
			Point(const double* coordinates, uint32_t dimension);
			Point(const Point& p);
			virtual ~Point();

			virtual Point& operator=(const Point& p);
			virtual bool operator==(const Point& p) const;

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
			virtual double GetMinimumDistance(const Point& p) const;

			virtual double GetCoordinate(uint32_t index) const;
			virtual double* GetCoordinates()const;
			virtual void SetDimension(const uint32_t d);

			
			friend class Region;

		private:
			uint32_t dimension;
			double* coordinates;

		private:

	};
}
#endif
