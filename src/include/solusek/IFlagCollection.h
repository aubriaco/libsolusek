/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_FLAG_COLLECTION_INCLUDED__
#define __I_FLAG_COLLECTION_INCLUDED__
#include "MFlag.h"

namespace solusek
{
	class IFlagCollection
	{
	public:
		virtual MFlag *get(EFlag flagId) = 0;
		virtual void add(MFlag flag) = 0;
		virtual unsigned int size() = 0;
		virtual void clear() = 0;
		virtual void dispose() = 0;
	};
}

#endif
