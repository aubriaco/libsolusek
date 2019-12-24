/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_ERROR_RESPONSE_INCLUDED__
#define __M_ERROR_RESPONSE_INCLUDED__
#include "MResponse.h"
#include <string>
#include "string_util.h"

namespace solusek
{
	class MErrorResponse : public MResponse
	{
	public:
		MErrorResponse(int code, const std::string &msg)
		{
			Code = code;
			char codeStr[16];
			sprintf(codeStr, "%i", code);
			Body = "{ \"code\": \"";
			Body += codeStr;
			Body += "\", \"message\": \"";
			Body += replaceAll(msg, "\"", "\\\"");
			Body += "\" }";
		}
	};
}

#endif
