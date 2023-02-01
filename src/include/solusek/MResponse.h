/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_RESPONSE_INCLUDED__
#define __M_RESPONSE_INCLUDED__
#include "MCookie.h"
#include <map>
#include <string>
#include <vector>

namespace solusek
{
	class MResponse
	{
	public:
		MResponse() { ContentType = "text/plain"; Code = 200; }
		MResponse(int code, const std::string &body, const std::string& sid = std::string(), const std::string& contentType = "text/plain") : Code(code), Body(body), ContentType(contentType), SID(sid) { ContentType = "text/plain"; }

		int Code;
		std::string Body, ContentType, Date, SID, Location;
    	std::vector<MCookie> Cookies;
		std::map<std::string, std::string> Headers;

		std::string getCodeDescription() const
		{
			switch(Code)
			{
				case 200:
					return "OK";
				case 404:
					return "Not Found";
				case 400:
					return "Bad Request";
				default:
					return "Unknown";
			}
		}
	};
}

#endif
