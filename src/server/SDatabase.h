/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __S_DATABASE_INCLUDED__
#define __S_DATABASE_INCLUDED__
#include <string>

namespace solusek
{
	struct SDatabase
	{
		std::string Host, User, Pass;
		int Port;
	};
}

#endif
