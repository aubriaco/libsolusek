/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_ENDPOINT_INCLUDED__
#define __M_ENDPOINT_INCLUDED__
#include <string>
#include "MRequest.h"
#include "MResponse.h"

namespace solusek
{
	class MEndpoint
	{
	public:
		MEndpoint() {}
		MEndpoint(const std::string &path, MResponse(*callback)(const void*, MRequest&), const std::string& method, const std::string& contentType = "text/plain")
		{
			Path = path;
			Method = method;
			Callback = callback;
            Type = 0;
            ContentType = contentType;
		}
    	int Type;
		std::string Path, Method, Contents, Ext, ContentType, Date;
		MResponse(*Callback)(const void*, MRequest&);
	};
}

#endif
