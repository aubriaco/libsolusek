/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_CONFIG_INCLUDED__
#define __I_CONFIG_INCLUDED__

namespace solusek
{
	class IConfig
	{
	public:
		virtual std::string get(const std::string& path) = 0;
	};
}

#endif
