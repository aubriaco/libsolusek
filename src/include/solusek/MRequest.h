/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_REQUEST_INCLUDED__
#define __M_REQUEST_INCLUDED__
#include "MCookie.h"
#include "IVarVal.h"
#include "string_util.h"
#include <vector>
#include <algorithm>

namespace solusek
{
	class MRequest
	{
	public:
		MRequest() {}
		MRequest(const std::string &body) : Body(body) {}
		std::vector<MCookie> Cookies;
		std::vector<IVarVal *> *Headers;
		unsigned int Length;
		std::string Body, Host, SID, QueryString;
		std::string SSLClientCN;

		MCookie getCookie(const std::string &name)
		{
			for (std::vector<MCookie>::iterator it = Cookies.begin(); it != Cookies.end(); ++it)
			{
				if ((*it).Name == name)
					return (*it);
			}
			return MCookie(true);
		}

		IVarVal *getHeader(const char *name)
		{
			std::string tmp = strtolower(name);
			std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
			for (std::vector<IVarVal *>::iterator it = Headers->begin(); it != Headers->end(); ++it)
			{
				if (strtolower((*it)->getVar()) == tmp)
					return (*it);
			}
			return 0;
		}
	};
}

#endif
