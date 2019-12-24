/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef CCLIENT_H
#define CCLIENT_H
#include "../include/solusek/IClient.h"
#include "../handlers/CNetHandler.h"
#include <map>

namespace solusek
{

	class CClient : public IClient
	{
	private:
		CNetHandler NH;
		CNetHandlerSocket *MainSocket;
		std::string Endpoint, Method;
		bool UseSSL, Verbose;
		std::string CertFileName, KeyFileName, KeyPassword, PKCS12FileName, PKCS12Password;
		void *CertMem, *KeyMem;
		std::map<std::string, std::string> AdditionalHeaders;
		int Timeout;
		CThreadHandler TH;
	public:
		CClient();
		~CClient();

		virtual void dispose();

		virtual void setTimeout(int timeout);

		virtual void setVerbose(bool b);

		virtual void setMethod(const std::string& s);

		virtual void setUseSSL(bool b);

		virtual void setEndpoint(const std::string &s);

		virtual void setSSLCertificate(const std::string& fn);

		virtual void setSSLKeyFile(const std::string& fn, const std::string& password);

		virtual void setSSLCertificateMem(void* fn);

		virtual void setSSLKeyFileMem(void* fn, const std::string& password);

		virtual void setSSLPKCS12(const std::string& fn, const std::string& password);

		virtual MResponse runRequest(const std::string& body, bool ignoreResult = false);

		virtual void setHeader(const std::string& name, const std::string& val);

		virtual void removeHeader(const std::string& name);
	};

}

#endif // CCLIENT_H
