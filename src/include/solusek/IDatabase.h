/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_DATABASE_INCLUDED__
#define __I_DATABASE_INCLUDED__
#include <map>
#include "MDatabaseHandle.h"
#include "IDatabaseInstance.h"
#include "ITransaction.h"

namespace solusek
{
	class IDatabase
	{
	public:
		virtual void setConnectionString(const std::string &cs) = 0;

		virtual IDatabaseInstance *open() = 0;
		virtual void closeAll() = 0;

		virtual std::string now() = 0;
	};
}

#endif
