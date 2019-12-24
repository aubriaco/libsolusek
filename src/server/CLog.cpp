/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CLog.h"

namespace solusek
{
	CLog::CLog()
	{
		// N/U
	}

	CLog::CLog(const std::string &prefix) : Prefix(prefix)
	{ }

	CLog::~CLog()
	{
		// N/U
	}

	void CLog::open()
	{
		// N/U
	}

	void CLog::close()
	{
		// N/U
	}

	void CLog::print(const std::string &s)
	{
		log_printf("%s", s.c_str());
	}
}
