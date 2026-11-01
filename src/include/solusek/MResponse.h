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
		MResponse(int code, const std::string &body, const std::string& sid = std::string(), const std::string& contentType = "text/plain") : Code(code), Body(body), ContentType(contentType), SID(sid) {}

		int Code;
		std::string Body, ContentType, Date, SID, Location;
    	std::vector<MCookie> Cookies;
		std::map<std::string, std::string> Headers;

		std::string getCodeDescription() const
		{
			switch(Code)
			{
				case 200: return "OK";
				case 201: return "Created";
				case 202: return "Accepted";
				case 204: return "No Content";
				case 301: return "Moved Permanently";
				case 302: return "Found";
				case 304: return "Not Modified";
				case 401: return "Unauthorized";
				case 403: return "Forbidden";
				case 405: return "Method Not Allowed";
				case 408: return "Request Timeout";
				case 413: return "Payload Too Large";
				case 415: return "Unsupported Media Type";
				case 429: return "Too Many Requests";
				case 400: return "Bad Request";
				case 404: return "Not Found";
				case 500: return "Internal Server Error";
				case 501: return "Not Implemented";
				case 503: return "Service Unavailable";
				default:
					return "Unknown";
			}
		}
	};
}

#endif
