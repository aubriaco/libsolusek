/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_NET_HANDLER_INCLUDED__
#define __C_NET_HANDLER_INCLUDED__

#include <string>
#include <vector>
#include "../include/solusek/INetHandlerSocket.h"
#include "CThreadHandler.h"

#ifdef __linux__
#	include <stdio.h>
#	include <string.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netdb.h>
#	define INVALID_SOCKET 0
#	define SOCKET_ERROR -1
#endif

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

static bool CNetHandler_SSLInitialized = false;


struct SNIObj
{
	std::string Host;
	void *Cert, *Key;
	std::vector<void*> Aux;
	bool Mutual;
};


class CNetHandlerSocket : public solusek::INetHandlerSocket
{
private:
	int				SID;
	bool			Ready, SSLEnabled, SSLMutual, ThrowExceptions;
	SSL_CTX			*CTX;
	SSL_CTX     *ChildCTX;
	SSL				*_SSL;
	SSL_METHOD		*Method;
	X509			*ClientCertificate;
	std::string		CertificateFileName, PrivateKeyFileName, CertificatePassword, ClientCAFileName;

	void setupSSL(bool re = false);
	void setupSSLSocket(int sid, bool accept = false, const char* host = 0);
	void cleanupSSL();
	std::string getSSLClientCertificatePublicKey();

	int Timeout;


	static int _sni_callback(SSL *ssl, int *i, void* param);
	bool SNIEnabled;
	std::string ClientIPAddr;

	std::vector<solusek::IPAddressItem> Blacklist, Whitelist;

	std::string SNIHostName;

	bool CTXRoot;
public:
	std::vector<SNIObj> SNIList;
	std::string CustomID, CustomUID, CustomPassword;
	int CustomStatus;
	unsigned long UniqueID;
	void* ProgramPackage;
	THREADHANDLE ThreadHandle;
	THREADHANDLE SupportThreadHandle;

	void setCTX(void *ctx);
	void setChildCTX(void *ctx);

	CNetHandlerSocket(int sid = 0, bool ssl = false, bool sslMutual = false, SSL_CTX* ctx = 0, SSL* _ssl = 0, const std::string& ipaddr = std::string());
	~CNetHandlerSocket();

	bool isReady() { return Ready; }

	std::string getClientIPAddress() { return ClientIPAddr; }

	void addBlacklist(const solusek::IPAddressItem& item) { Blacklist.push_back(item); }
	void addWhitelist(const solusek::IPAddressItem& item) { Whitelist.push_back(item); }

	bool isBlacklisted(const std::string& addr);
	bool isWhitelisted(const std::string& addr);

	void clearBlacklist() { Blacklist.empty(); }
	void clearWhitelist() { Whitelist.empty(); }

	void setSNIHostName(const std::string& name) { SNIHostName = name; }
	std::string getSNIHostName() { return SNIHostName; }

	// SSL
	bool setSSLCertificateMem(void *data);
	bool setSSLPrivateKeyFileMem(void* data);
	bool setSSLCertificate(const char* filename);
	bool setSSLPrivateKeyFile(const char* filename);
	void setSSLCertificatePassword(const char* password);
	bool setSSLClientCAFile(const char* filename);
	void addSSLCertificateToSNI(const char* host, const char* filename, const char* keyfilename = 0, bool mutual = true);
	bool isSSLValid();
	void setSSLMutual(bool enabled);
	void mutexSSL();
	solusek::CertificateInfo getClientCertificate();
	void* openClientCertificateX509();
	void closeClientCertificateX509();

	void setSNIEnabled(bool enabled);
	void setSNIClientHost(const std::string& host);
	void cleanupSNI();

	void setThrowExceptions(bool enabled) { ThrowExceptions = enabled; }

	bool bind(int port, const char* host = 0);
	int listen();
	CNetHandlerSocket* accept();
	int readBuffer(void *buf, int len);
	bool writeBuffer(const void* buf, int len);
	bool connect(const char* host, int port);
	bool close();

	void setTimeout(int timeout);
	int getTimeout() { return Timeout; }

	bool isConnected();

	void dispose() { delete this; }
};

class CNetHandler
{
private:

#ifdef _WIN32
	WSADATA WD;
#endif

public:
#ifdef _WIN32
	CNetHandler() { WSAStartup( MAKEWORD( 2, 0 ), & WD ); };
	~CNetHandler() { WSACleanup();  };
#elif __linux__
	CNetHandler() {};
	~CNetHandler() {};
#endif

	CNetHandlerSocket* createSocket(bool ssl = false);


	static inline void initDependencies()
	{
		if(!CNetHandler_SSLInitialized)
		{
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
            CNetHandler_SSLInitialized = true;
		}
	}
};


#endif
