/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CNetHandler.h"
#include <cstring>
#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#endif

using namespace solusek;

#ifdef _WIN32
namespace
{
	class WinsockRuntime
	{
	public:
		WinsockRuntime()
		{
			WSADATA data;
			if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
				throw std::runtime_error("Winsock initialization failed");
		}

		~WinsockRuntime() { WSACleanup(); }
	};
}
#endif

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

CNetHandlerSocket *CNetHandler::createSocket(bool ssl)
{
	return new CNetHandlerSocket(INVALID_SOCKET, ssl);
}

CNetHandlerSocket::CNetHandlerSocket(SocketHandle sid, bool ssl, bool sslMutual, SSL_CTX *ctx, SSL *_ssl, const std::string &ipaddr)
{
#ifdef _WIN32
	static WinsockRuntime winsock;
#endif
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
	if (!ipaddr.empty())
		ClientIPAddr = ipaddr;
	if (SSLEnabled && !_SSL)
		setupSSL();
	UniqueID = 0;
	Ready = true;
	if (sid == INVALID_SOCKET)
	{
		SID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (SID == INVALID_SOCKET)
			Ready = false;
		else
		{
			int val = 1;
			int nval = 0;
#ifdef _WIN32
			setsockopt(SID, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, reinterpret_cast<const char *>(&val), sizeof(val));
#else
			setsockopt(SID, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
#endif
			setsockopt(SID, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&nval), sizeof(nval));
#ifdef _WIN32
			const DWORD timeout = 10000;
#else
			const timeval timeout = {10, 0};
#endif
			setsockopt(SID, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&timeout), sizeof(timeout));
			// int set = 1;
			// setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
			Ready = true;
		}
	}
	else
		SID = sid;
}

CNetHandlerSocket::~CNetHandlerSocket()
{
	close();
	if (SSLEnabled)
		cleanupSSL();
}

void CNetHandlerSocket::setTimeout(int timeout)
{
	Timeout = timeout;
#ifdef _WIN32
	const DWORD tv = static_cast<DWORD>(timeout) * 1000;
#else
	const timeval tv = {timeout, 0};
#endif
	setsockopt(SID, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv));
	setsockopt(SID, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv));
}

bool CNetHandlerSocket::bind(int port, const char *host)
{
	sockaddr_in addr = {};
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

bool isAddrInRange(const std::string &addr, const std::string &from, const std::string &to)
{
	unsigned int np = from.find_last_of('.');
	if (np == std::string::npos)
		return false;
	std::string prefix(from.substr(0, np));

	if (to.size() < np)
		return false;

	std::string addrPrefix(addr.substr(0, np));

	if (prefix != addrPrefix)
		return false;

	short fromi = atoi(from.substr(np + 1).c_str());
	short toi = atoi(to.substr(np + 1).c_str());
	short isi = atoi(addr.substr(np + 1).c_str());

	if (isi <= toi && isi >= fromi)
		return true;

	return false;
}

bool CNetHandlerSocket::isBlacklisted(const std::string &addr)
{
	for (std::vector<IPAddressItem>::iterator it = Blacklist.begin(); it != Blacklist.end(); ++it)
	{
		if ((*it).Addr == addr || isAddrInRange(addr, (*it).From, (*it).To))
			return true;
	}
	return false;
}

bool CNetHandlerSocket::isWhitelisted(const std::string &addr)
{
	for (std::vector<IPAddressItem>::iterator it = Whitelist.begin(); it != Whitelist.end(); ++it)
	{
		if ((*it).Addr == addr || isAddrInRange(addr, (*it).From, (*it).To))
			return true;
	}
	return false;
}

CNetHandlerSocket *CNetHandlerSocket::accept()
{
	sockaddr clientAddress;
	memset(&clientAddress, 0, sizeof(sockaddr));
#ifdef _WIN32
	int clientLength = sizeof(sockaddr);
	const SocketHandle sid = ::accept(SID, &clientAddress, &clientLength);
#else
	socklen_t clientLength = sizeof(sockaddr);
	SocketHandle sid;
	while ((sid = ::accept(SID, (sockaddr *)&clientAddress, &clientLength)) == SOCKET_ERROR && errno == EINTR)
	{
	}
#endif
	if (sid == INVALID_SOCKET)
		return 0;

	char ipaddr[INET6_ADDRSTRLEN] = {};
	inet_ntop(clientAddress.sa_family, get_in_addr((struct sockaddr *)&clientAddress), ipaddr, sizeof(ipaddr));

	if (!isWhitelisted(ipaddr) && isBlacklisted(ipaddr))
	{
#ifdef _WIN32
		::closesocket(sid);
#else
		::close(sid);
#endif
		return 0;
	}

	if (SSLEnabled)
		setupSSLSocket(sid, true);

	CNetHandlerSocket *sock = new CNetHandlerSocket(sid, SSLEnabled, SSLMutual, ChildCTX ? ChildCTX : CTX, _SSL, ipaddr);
	_SSL = 0;
	if (SSLEnabled)
		sock->setSNIHostName(SNIHostName);

	return sock;
}

int CNetHandlerSocket::readBuffer(void *buf, int len)
{
	int r;
	if (SSLEnabled)
		r = ::SSL_read(_SSL, (char *)buf, len);
	else
		r = ::recv(SID, (char *)buf, len, 0);
	if (ThrowExceptions && r < 1)
		throw (int)r;
	return r;
}

bool CNetHandlerSocket::connect(const char *host, int port)
{
	sockaddr_in addr;

	struct addrinfo *result;

	addrinfo hints = {};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int gai = getaddrinfo(host, nullptr, &hints, &result);

	if (gai != 0)
		return false;

	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	memcpy((char *)&addr.sin_addr, (char *)&((struct sockaddr_in *)result->ai_addr)->sin_addr, sizeof(struct in_addr));
	addr.sin_port = htons(port);

	freeaddrinfo(result);

	int r = ::connect(SID, (struct sockaddr *)&addr, sizeof(addr));

	if (r == 0 && SSLEnabled)
		setupSSLSocket(SID, false, host);

	return (r == 0);
}

bool CNetHandlerSocket::writeBuffer(const void *buf, int len)
{
	if (len < 0 || (len > 0 && !buf) || SID == INVALID_SOCKET)
		return false;
	const char *data = static_cast<const char *>(buf);
	int offset = 0;
	while (offset < len)
	{
		int written;
		if (SSLEnabled)
			written = ::SSL_write(_SSL, data + offset, len - offset);
		else
		{
#ifdef _WIN32
			written = ::send(SID, data + offset, len - offset, 0);
			if (written == SOCKET_ERROR && WSAGetLastError() == WSAEINTR)
				continue;
#else
			written = static_cast<int>(::send(SID, data + offset,
											  static_cast<size_t>(len - offset), MSG_NOSIGNAL));
			if (written < 0 && errno == EINTR)
				continue;
#endif
		}
		if (written <= 0)
		{
			if (ThrowExceptions)
				throw written;
			return false;
		}
		offset += written;
	}
	return true;
}

bool CNetHandlerSocket::close()
{
	if (SID != INVALID_SOCKET)
	{
#ifdef _WIN32
		::shutdown(SID, SD_BOTH);
		::closesocket(SID);
#else
		::shutdown(SID, SHUT_RDWR);
		::close(SID);
#endif
		SID = INVALID_SOCKET;
	}
	return true;
}
