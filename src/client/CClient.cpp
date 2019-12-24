/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CClient.h"
#include <map>
#ifdef USE_OPENSSL
#include <openssl/pkcs12.h>
#endif
#include <algorithm>
#include <cctype>
#include <locale>
#include "../include/solusek/string_util.h"

namespace solusek
{
	CClient::CClient()
	{
		UseSSL = false;
		Verbose = false;
		MainSocket = 0;
		Method = "POST";
		KeyMem = 0;
		CertMem = 0;
		AdditionalHeaders["Content-type"] = "application/json";
		Timeout = 30000;
	}

	CClient::~CClient()
	{
	}

	void CClient::dispose()
	{
		delete this;
	}

	void CClient::setTimeout(int timeout)
	{
		Timeout = timeout;
	}

	void CClient::setVerbose(bool b)
	{
		Verbose = b;
	}

	void CClient::setMethod(const std::string& s)
	{
		Method = s;
	}

	void CClient::setUseSSL(bool b)
	{
		UseSSL = b;
	}

	void CClient::setEndpoint(const std::string &s)
	{
		Endpoint = s;
	}

	void CClient::setHeader(const std::string& name, const std::string& val)
	{
		AdditionalHeaders[name] = val;
	}

	void CClient::removeHeader(const std::string& name)
	{
		AdditionalHeaders.erase(name);
	}

	void CClient::setSSLCertificate(const std::string& fn)
	{
		CertFileName = fn;
	}

	void CClient::setSSLKeyFile(const std::string& fn, const std::string& password)
	{
		KeyFileName = fn;
		KeyPassword = password;
	}

	void CClient::setSSLCertificateMem(void* fn)
	{
#ifdef USE_OPENSSL
		if(CertMem)
			X509_free((X509*)CertMem);
#endif
		CertMem = fn;
	}

	void CClient::setSSLKeyFileMem(void* fn, const std::string& password)
	{
#ifdef USE_OPENSSL
		if(KeyMem)
			EVP_PKEY_free((EVP_PKEY*)KeyMem);
#endif
		KeyMem = fn;
		KeyPassword = password;
	}

	void CClient::setSSLPKCS12(const std::string& fn, const std::string& password)
	{
		PKCS12FileName = fn;
		PKCS12Password = password;
	}

	MResponse CClient::runRequest(const std::string& body, bool ignoreResult)
	{
		std::string contentType("application/json"), location;
		int port = 80;

		std::size_t n1 = Endpoint.find("://");

		if(n1 == std::string::npos)
		{
			fprintf(stdout,"Invalid endpoint.\n");
			return MResponse();
		}

		std::string protocol = Endpoint.substr(0, n1);

		if(protocol == "https")
		{
			port = 443;
			UseSSL = true;
		}

		std::string host = Endpoint.substr(n1 + 3);

		std::size_t nt = host.find("/");
		n1 = host.find("@");
		if(n1 != std::string::npos && n1 < nt)
		{
			std::string auth = host.substr(0, n1);
			host = host.substr(n1 + 1);
			//setHeader("Authorization", "Basic " + base64encode((unsigned char*)auth.c_str(), auth.size()));
			if(Verbose)
				fprintf(stdout,"Using authorization header.\n");
		}




		std::string path("/");

		std::size_t n2 = host.find(":");
		if(n2 != std::string::npos)
		{
			std::string sport = host.substr(n2 + 1);
			host = host.substr(0, n2);
			std::size_t n3 = sport.find('/');
			if(n3 != std::string::npos)
			{
				path = sport.substr(n3);
				sport = sport.substr(0, n3);
			}
			port = atoi(sport.c_str());
		}
		else
		{
			n2 = host.find("/");
			if(n2 != std::string::npos)
			{
				path = host.substr(n2);
				host = host.substr(0, n2);
			}
		}

		if(UseSSL && Verbose)
			fprintf(stdout,"Using SSL\n");

		if(Verbose)
			fprintf(stdout,"HOST: %s\nPORT: %i\nPATH: %s\n", host.c_str(), port, path.c_str());

		MainSocket = NH.createSocket(UseSSL);
		MainSocket->setThrowExceptions(true);

		if(CertFileName.size() > 0)
		{
			MainSocket->setSSLCertificate(CertFileName.c_str());
		}

		if(KeyFileName.size() > 0)
		{
			MainSocket->setSSLPrivateKeyFile(KeyFileName.c_str());
			MainSocket->setSSLCertificatePassword(KeyPassword.c_str());
		}


		if(PKCS12FileName.size() > 0)
		{
			FILE* file = fopen(PKCS12FileName.c_str(), "rb");

			if(file)
			{
#ifdef USE_OPENSSL
				PKCS12* pf;
				pf = d2i_PKCS12_fp(file, 0);

				EVP_PKEY* pkey = 0;
				X509* cert = 0;
				STACK_OF(X509) *ca = 0;

				int r = PKCS12_parse(pf, PKCS12Password.c_str(), &pkey, &cert, &ca);

				if(r == 1)
				{
					MainSocket->setSSLCertificateMem(cert);
					MainSocket->setSSLPrivateKeyFileMem(pkey);
				}
#endif
				fclose(file);
			}
		}

		if(CertMem)
			MainSocket->setSSLCertificateMem(CertMem);
		if(KeyMem)
			MainSocket->setSSLPrivateKeyFileMem(KeyMem);

		if(UseSSL)
		{
			MainSocket->mutexSSL();
			MainSocket->setSSLMutual(true);
		}

		int code = 0;
		std::string resp;

		try
		{
			MainSocket->setTimeout(Timeout);
			MainSocket->connect(host.c_str(), port);

			if(Verbose)
				fprintf(stdout,"Connected!\n");

			std::string data(Method);
			data += " ";
			data += path;
			data += " HTTP/1.1";
			data += "\r\n";
			data += "Host: ";
			data += host;
			data += "\r\n";
			for(std::map<std::string, std::string>::iterator it = AdditionalHeaders.begin(); it != AdditionalHeaders.end(); ++it)
			{
				data += (*it).first;
				data += ": ";
				data += (*it).second;
				data += "\r\n";
			}
			//data += "Content-type: application/json\r\n";
			if(body.size() > 0)
			{
				data += "Content-length: ";
				char buf[16];
				sprintf(buf, "%lu", body.size());
				data += buf;
				data += "\r\n";
			}
			data += "\r\n";

			if(Verbose)
			{
				fprintf(stdout,"Body size: %lu\n", body.size());
				fprintf(stdout,"HEADERS:\n%s\n", data.c_str());
			}

			MainSocket->writeBuffer(data.c_str(), data.size());

			int chunk = 1024;
			unsigned long int done = 0;
			while(done < body.size())
			{
				if(done + chunk > body.size())
					chunk = body.size() - done;
				MainSocket->writeBuffer(&body[0] + done, chunk);
				done += chunk;
				if(Verbose)
					fprintf(stdout,"%lu/%lu\n", done, body.size());
			}


			if(Verbose)
				fprintf(stdout,"Sent request!\n");

			if(!ignoreResult)
			{

				std::map<std::string, std::string> headers;
				char retBuf[2];
				int ex = 0;
				std::string line;
				while(true)
				{

					MainSocket->readBuffer(retBuf, 1);
					retBuf[1] = 0;


					if(retBuf[0] == '\r')
						continue;
					if(retBuf[0] == '\n')
					{
						if(line.find("HTTP/1.1") != std::string::npos)
						{
							int i1 = line.find(" ");
							std::string scode = line.substr(i1+1);
							int i2 = scode.find(" ");
							scode = scode.substr(0, i2);
							code = atoi(scode.c_str());
						}
						else
						{

							std::string var, val;
							int i1 = line.find(":");
							var = strtolower(line.substr(0, i1));
							val = line.substr(i1+1);

							trim(var);
							trim(val);
							headers[var] = val;
						}
						if(Verbose)
							fprintf(stdout,"LINE: %s\n", line.c_str());
						line = "";
						ex++;
						if(ex == 2)
							break;
					}
					else
					{
						line += retBuf;
						ex = 0;
					}

				}

				if(headers["content-type"].size() > 0)
				{
					contentType = headers["content-type"];
				}

				if(headers["content-length"].size() > 0)
				{
					int clen = atoi(headers["content-length"].c_str());

					int r = 0;
					for(unsigned int n = 0; n < clen; n++)
					{
						r = MainSocket->readBuffer(retBuf, 1);
						retBuf[r] = 0;
						//resp += retBuf;
						if(r > 0)
							resp.insert(resp.end(), retBuf[0]);
					}

				}
				else if(headers["transfer-encoding"].size() > 0)
				{
					std::string acc;
					unsigned int chunkSize;
					int r = 0;
					char bbuf[2], *vbuf;
					try
					{
						memset(bbuf, 0, 2);
						while(true)
						{
							char prev = 0;
							acc = std::string();
							while(true)
							{
								r = MainSocket->readBuffer(bbuf, 1);
								if(r < 1)
									throw -1;
								bbuf[r] = 0;
								if(prev == '\r' && bbuf[0] == '\n')
									break;
								if(bbuf[0] != '\r')
									acc.insert(acc.end(), bbuf[0]);
								prev = bbuf[0];
							}

							if(Verbose)
								fprintf(stdout,"CHUNK HEX: %s\n", acc.c_str());

							sscanf(acc.c_str(), "%x", &chunkSize);

							if(Verbose)
								fprintf(stdout,"CHUNK SIZE: %u\n", chunkSize);

							if(chunkSize == 0)
								throw -1;

							char cbuf[1];
							memset(cbuf, 0, 1);
							for(unsigned int n = 0; n < chunkSize; n++)
							{
								r = MainSocket->readBuffer(cbuf, 1);
								if(r > 0)
									resp.insert(resp.end(), cbuf[0]);
							}
							if(Verbose)
								fprintf(stdout,"ACCUM: %s\n", &resp[1]);
							MainSocket->readBuffer(bbuf, 2);
							if(Verbose)
								fprintf(stdout,"RECV: %lu\n", resp.size());
						}
					}
					catch(int ii)
					{
						if(Verbose)
							fprintf(stdout,"DONE\n");
					}
					if(Verbose)
						fprintf(stdout,"RESP: %s\n", resp.c_str());
					MainSocket->setTimeout(Timeout);
				}
				if(Verbose)
					fprintf(stdout,"%s\n", resp.c_str());

				if(headers["location"].size() > 0)
				{
					location = headers["location"];
				}

			}

		}
		catch(int i)
		{
			fprintf(stdout,"Socket error: %i\n", i);
		}

		delete MainSocket;

		MResponse ret(code, resp);
		ret.ContentType = contentType;
		ret.Location = location;
		return ret;
	}
}
