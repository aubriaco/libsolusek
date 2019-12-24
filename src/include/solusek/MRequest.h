/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_REQUEST_INCLUDED__
#define __M_REQUEST_INCLUDED__
#include "MCookie.h"
#include <vector>

namespace solusek
{
	class MRequest
	{
	public:
		MRequest() {}
		MRequest(const std::string &body) : Body(body) {}
		std::vector<MCookie> Cookies;
		unsigned int Length;
		std::string Body, Host, SID;
		std::string SSLClientCN;

		MCookie getCookie(const std::string& name)
		{
			for(std::vector<MCookie>::iterator it = Cookies.begin(); it != Cookies.end(); ++it)
			{
				if((*it).Name == name)
					return (*it);
			}
			return MCookie(true);
		}
	};
}

#endif
