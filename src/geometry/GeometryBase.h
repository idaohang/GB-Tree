/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
#ifndef GEOMETRY_BASE_H_
#define GEOMETRY_BASE_H_
#include "../util/ProgException.h"
#include "../util/Tools.h"
#include "../base/Interfaces.h"
namespace Geometry{

	class IGeometry : public Indexes::ISerializable
	{
		public:
			virtual bool IntersectGeometry(const IGeometry& input) const = 0;
			virtual bool ContainGeometry(const IGeometry& input) const = 0;
			virtual bool TouchGeometry(const IGeometry& input) const = 0;
			virtual void GetCenter(Point& out) const = 0;
			virtual uint32_t GetDimesion() const = 0;
			virtual void GetMBR(Region& out) const = 0;
			virtual double GetArea() const = 0;
			virtual double GetMinimumDistance(const IGeometry& input) const = 0;
			virtual ~IGeometry() {}
	};
}
#endif
