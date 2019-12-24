/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef ICLIENT_H
#define ICLIENT_H
#include <string>
#include "MResponse.h"

namespace solusek
{

	class IClient
	{
	public:

		virtual void dispose() = 0;

		virtual void setTimeout(int timeout) = 0;

		virtual void setVerbose(bool b) = 0;

		virtual void setMethod(const std::string& s)= 0;

		virtual void setUseSSL(bool b) = 0;

		virtual void setEndpoint(const std::string &s) = 0;

		virtual void setSSLCertificate(const std::string& fn) = 0;

		virtual void setSSLKeyFile(const std::string& fn, const std::string& password) = 0;

		virtual void setSSLCertificateMem(void* fn) = 0;

		virtual void setSSLKeyFileMem(void* fn, const std::string& password) = 0;

		virtual void setSSLPKCS12(const std::string& fn, const std::string& password) = 0;

		virtual MResponse runRequest(const std::string& body, bool ignoreResult = false) = 0;

		virtual void setHeader(const std::string& name, const std::string& val) = 0;

		virtual void removeHeader(const std::string& name) = 0;

	};

}

#endif // ICLIENT_H
