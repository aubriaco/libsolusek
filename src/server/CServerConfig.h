/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_SERVER_CONFIG_INCLUDED__
#define __C_SERVER_CONFIG_INCLUDED__
#include <string>

namespace solusek
{
	class CServerConfig
	{
	public:
		std::string Path;

		int Port;
	}
}

#endif
