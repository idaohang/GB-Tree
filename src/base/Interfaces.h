/*
 * Copyright (C) 2014 by Liu Qi at Wuhan University
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Edward Liou <Liou AT liuqi.edward@gmail.com>
 * @date 5/2/2014
 */
#ifndef INDEXES_INTERFACES_H_
#define INDEXES_INTERFACES_H_
#include "../util/Tools.h"
namespace indexes
{
	class ISerializable
	{
		public:
			virtual ~ISerializable();
	
			virtual uint32_t GetByteArraySize() = 0;//return the size of bytes
	
			virtual void LoadFromByteArray(const byte* data) = 0;
	
			virtual void StoreToByteArray(byte** data, uint32_t&length) = 0;
	};
}
#endif
