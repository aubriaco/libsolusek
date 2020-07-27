/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifdef __linux__
#include "CNetHandler.h"
#include <unistd.h>
#include <arpa/inet.h>

using namespace solusek;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

CNetHandlerSocket* CNetHandler::createSocket(bool ssl)
{
	return new CNetHandlerSocket(0, ssl);
}

#ifdef USE_OPENSSL
CNetHandlerSocket::CNetHandlerSocket(int sid, bool ssl, bool sslMutual, SSL_CTX* ctx, SSL* _ssl, const char* ipaddr)
#endif
{
	Timeout = 0;
	_SSL = _ssl;
	CTX = ctx;
	ChildCTX = 0;
	CTXRoot = ctx ? false : true;
	SSLEnabled = ssl;
	SSLMutual = sslMutual;
	ThrowExceptions = false;
	ClientCertificate = 0;
	SNIEnabled = false;
	if(ipaddr)
		ClientIPAddr = ipaddr;
	if(SSLEnabled && !_SSL)
		setupSSL();
	UniqueID = 0;
	if(sid == 0)
	{
		SID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);


		if(SID == INVALID_SOCKET)
			Ready = false;
		else
		{
			int val = 1;
			int nval = 0;
			setsockopt(SID, SOL_SOCKET, SO_REUSEADDR, (const void*)&val, sizeof(val));
			setsockopt(SID, SOL_SOCKET, SO_KEEPALIVE, (const void*)&nval, sizeof(nval));
			struct timeval timeout;
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;
			setsockopt(SID, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));
			//int set = 1;
			//setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
			Ready = true;
		}
	}
	else
		SID = sid;
}

CNetHandlerSocket::~CNetHandlerSocket()
{
  	close();
  	if(SSLEnabled)
    	cleanupSSL();
}

void CNetHandlerSocket::setTimeout(int timeout)
{
	Timeout = timeout;
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(SID, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	setsockopt(SID, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
}

bool CNetHandlerSocket::bind(int port, const char* host)
{
	sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = host ? inet_addr(host) : INADDR_ANY;
	addr.sin_port = htons(port);
	int r = ::bind(SID, (sockaddr *)&addr, sizeof(addr));

	return (r == 0);
}

int CNetHandlerSocket::listen()
{
	return ::listen(SID, SOMAXCONN);
}

bool isAddrInRange(const std::string& addr, const std::string& from, const std::string& to)
{
  unsigned int np = from.find_last_of('.');
	if(np == std::string::npos)
		return false;
	std::string prefix(from.substr(0, np));

	if(to.size() < np)
		return false;

	std::string addrPrefix(addr.substr(0,np));

	if(prefix != addrPrefix)
		return false;

	short fromi = atoi(from.substr(np+1).c_str());
	short toi = atoi(to.substr(np+1).c_str());
	short isi = atoi(addr.substr(np+1).c_str());

	if(isi <= toi && isi >= fromi)
		return true;

	return false;
}

bool CNetHandlerSocket::isBlacklisted(const std::string& addr)
{
	for(std::vector<IPAddressItem>::iterator it = Blacklist.begin(); it != Blacklist.end(); ++it)
	{
		if((*it).Addr == addr || isAddrInRange(addr, (*it).From, (*it).To))
			return true;
	}
	return false;
}

bool CNetHandlerSocket::isWhitelisted(const std::string& addr)
{
	for(std::vector<IPAddressItem>::iterator it = Whitelist.begin(); it != Whitelist.end(); ++it)
	{
		if((*it).Addr == addr || isAddrInRange(addr, (*it).From, (*it).To))
			return true;
	}
	return false;
}


CNetHandlerSocket* CNetHandlerSocket::accept()
{
	sockaddr clientAddress;
	memset(&clientAddress, 0, sizeof(sockaddr));
	socklen_t clientLength = sizeof(sockaddr);
	int sid = 0;
	while((sid = ::accept(SID, (sockaddr *) &clientAddress, &clientLength)) == SOCKET_ERROR);

	char ipaddr[INET6_ADDRSTRLEN];
	inet_ntop(clientAddress.sa_family, get_in_addr((struct sockaddr *)&clientAddress), ipaddr, sizeof(ipaddr));

	if(!isWhitelisted(ipaddr) && isBlacklisted(ipaddr))
	{
		::close(sid);
		return 0;
	}


	if(SSLEnabled)
		setupSSLSocket(sid,true);

	CNetHandlerSocket* sock = new CNetHandlerSocket(sid, SSLEnabled, SSLMutual, ChildCTX ? ChildCTX : CTX, _SSL, ipaddr);
  _SSL = 0;
  if(SSLEnabled)
    sock->setSNIHostName(SNIHostName);

	return sock;
}

int CNetHandlerSocket::readBuffer(void* buf, int len)
{
	int r;
	if(SSLEnabled)
#ifdef USE_OPENSSL
		r = ::SSL_read(_SSL, (char*)buf, len);
#endif
	else
		r = ::recv(SID, (char *)buf, len, 0);
	if(ThrowExceptions && r < 1)
		throw (int)r;
	return r;
}

bool CNetHandlerSocket::connect(const char* host, int port)
{
	sockaddr_in addr;

  struct addrinfo *result;

  int gai = getaddrinfo(host, NULL, 0, &result);

  if(gai != 0)
    return false;

	memset((char *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	memcpy((char*) &addr.sin_addr, (char*)&((struct sockaddr_in *) result->ai_addr)->sin_addr, sizeof(struct in_addr));
	addr.sin_port = htons(port);

  freeaddrinfo(result);

	int r =::connect(SID, (struct sockaddr *)&addr, sizeof(addr));

	if(r == 0 && SSLEnabled)
		setupSSLSocket(SID,false,host);

	return (r == 0);
}

bool CNetHandlerSocket::writeBuffer(const void* buf, int len)
{
	int r = 0;
	if(SSLEnabled)
#ifdef USE_OPENSSL
		r = ::SSL_write(_SSL, (const char*)buf, len);
#endif
	else
		r = ::send(SID, (const char*) buf, len, MSG_NOSIGNAL);

	if(ThrowExceptions && r < 1)
		throw r;
	return (r >= 0);
}


bool CNetHandlerSocket::close()
{
	shutdown(SID,2);
	::close(SID);

	return true;
}

#endif
