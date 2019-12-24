/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_LOG_INCLUDED__
#define __C_LOG_INCLUDED__
#include <stdio.h>
#include <string>
#include <stdarg.h>

namespace solusek
{
	class CLog
	{
	private:
		std::string Prefix;
	public:
		CLog();
		CLog(const std::string &prefix);
		~CLog();
		virtual void setPrefix(const std::string &prefix) { Prefix = prefix; }
		virtual void open();
		virtual void close();
		virtual void print(const std::string &s);
        static int log_printf(const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			int rc = vfprintf(stdout, fmt, args);
			va_end(args);
			fflush(stdout);
			return rc;
		}
	};
}

#endif
