/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_DATABASE_HANDLE_INCLUDED__
#define __M_DATABASE_HANDLE_INCLUDED__
#include <string>
#include <vector>


namespace solusek
{
	class MDatabaseHandle
	{
	public:
		virtual std::string getID() = 0;

		virtual void setID(const std::string& id) = 0;

		virtual std::vector< std::vector<std::string> > getData() = 0;

		virtual unsigned int size() = 0;

		virtual void dispose() = 0;

		virtual std::string disposeAndGetError() = 0;

		virtual std::string disposeAndGetID() = 0;

		virtual bool isError() = 0;

		virtual std::string getError() = 0;
	};
}

#endif
