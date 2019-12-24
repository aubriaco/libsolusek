/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_RESPONSE_INCLUDED__
#define __M_RESPONSE_INCLUDED__
#include "MCookie.h"
#include <string>
#include <vector>

namespace solusek
{
	class MResponse
	{
	public:
		MResponse() { ContentType = "application/json"; Code = 200; }
		MResponse(int code, const std::string &body, const std::string& sid = std::string()) : Code(code), Body(body), SID(sid) { ContentType = "application/json"; }

		int Code;
		std::string Body, ContentType, Date, SID, Location;
    std::vector<MCookie> Cookies;

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
