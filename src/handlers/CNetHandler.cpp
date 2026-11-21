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
#include <memory>
#include <algorithm>
#include <climits>
#include <system_error>
#include <openssl/err.h>

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

	if ((!isWhitelisted(ipaddr) && isBlacklisted(ipaddr)) ||
		(SSLEnabled && !setupSSLSocket(sid, true)))
	{
#ifdef _WIN32
		::closesocket(sid);
#else
		::close(sid);
#endif
		return 0;
	}

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

	if (r != 0)
		return false;

	if (SSLEnabled && !setupSSLSocket(SID, false, host))
	{
		close();
		return false;
	}

	return true;
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

namespace
{
	// Wait until a nonblocking TLS operation can be retried without spinning.
	void waitForTLS(SocketHandle sid, int sslError)
	{
		fd_set readSet, writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);

		if (sslError == SSL_ERROR_WANT_READ)
			FD_SET(sid, &readSet);
		else
			FD_SET(sid, &writeSet);

		int result;
		do
		{
			// select modifies its sets, so reconstruct them after interruption.
			FD_ZERO(&readSet);
			FD_ZERO(&writeSet);
			if (sslError == SSL_ERROR_WANT_READ)
				FD_SET(sid, &readSet);
			else
				FD_SET(sid, &writeSet);
#ifdef _WIN32
			result = ::select(0, &readSet, &writeSet, nullptr, nullptr);
#else
			result = ::select(sid + 1, &readSet, &writeSet, nullptr, nullptr);
#endif
		}
#ifndef _WIN32
		while (result < 0 && errno == EINTR);
#else
		while (false);
#endif

		if (result <= 0)
			throw std::runtime_error("TLS socket readiness wait failed.");
	}

	[[noreturn]] void throwTLSError(const char *operation, int sslError, int savedErrno)
	{
		if (sslError == SSL_ERROR_ZERO_RETURN)
			throw std::runtime_error(std::string(operation) + ": TLS connection closed by peer.");

		if (sslError == SSL_ERROR_SYSCALL && savedErrno != 0)
			throw std::system_error(savedErrno, std::generic_category(), operation);

		const unsigned long code = ERR_get_error();
		if (code)
		{
			char message[256];
			ERR_error_string_n(code, message, sizeof(message));
			throw std::runtime_error(std::string(operation) + ": " + message);
		}
		throw std::runtime_error(std::string(operation) + ": TLS operation failed (SSL error " +
								 std::to_string(sslError) + ").");
	}
}

void CNetHandlerSocket::readExact(void *data, size_t size)
{
	auto ptr = static_cast<unsigned char *>(data);

	while (size > 0)
	{
		if (SSLEnabled)
		{
			if (!_SSL)
				throw std::runtime_error("TLS connection is not initialized.");

			// OpenSSL's _ex API accepts size_t and reports actual bytes transferred.
			size_t received = 0;
			ERR_clear_error();
			errno = 0;
			const int result = SSL_read_ex(_SSL, ptr, size, &received);
			const int savedErrno = errno;

			if (result == 1)
			{
				if (received == 0)
					throw std::runtime_error("TLS read made no progress.");
				ptr += received;
				size -= received;
				continue;
			}

			const int error = SSL_get_error(_SSL, result);
			if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
			{
				waitForTLS(SID, error);
				continue;
			}
			throwTLSError("SSL_read_ex", error, savedErrno);
		}

#ifdef _WIN32
		const int n = ::recv(SID, reinterpret_cast<char *>(ptr),
							 static_cast<int>(std::min(size, static_cast<size_t>(INT_MAX))), 0);
		if (n == SOCKET_ERROR)
		{
			const int error = WSAGetLastError();
			if (error == WSAEINTR)
				continue;
			throw std::runtime_error("recv failed (Winsock error " + std::to_string(error) + ").");
		}
#else
		const size_t n = ::recv(SID, ptr, size, 0);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::system_error(errno, std::generic_category(), "recv");
		}
#endif
		if (n == 0)
			throw std::runtime_error("Connection closed by peer.");
		ptr += n;
		size -= static_cast<size_t>(n);
	}
}

void CNetHandlerSocket::writeExact(const void *data, size_t size)
{
	auto ptr = static_cast<const unsigned char *>(data);

	while (size > 0)
	{
		if (SSLEnabled)
		{
			if (!_SSL)
				throw std::runtime_error("TLS connection is not initialized.");

			size_t written = 0;
			ERR_clear_error();
			errno = 0;
			const int result = SSL_write_ex(_SSL, ptr, size, &written);
			const int savedErrno = errno;

			if (result == 1)
			{
				if (written == 0)
					throw std::runtime_error("TLS write made no progress.");
				ptr += written;
				size -= written;
				continue;
			}

			const int error = SSL_get_error(_SSL, result);
			if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE)
			{
				// Keep the same pointer and length until the write completes.
				waitForTLS(SID, error);
				continue;
			}
			throwTLSError("SSL_write_ex", error, savedErrno);
		}

#ifdef _WIN32
		const int n = ::send(SID, reinterpret_cast<const char *>(ptr),
							 static_cast<int>(std::min(size, static_cast<size_t>(INT_MAX))), 0);
		if (n == SOCKET_ERROR)
		{
			const int error = WSAGetLastError();
			if (error == WSAEINTR)
				continue;
			throw std::runtime_error("send failed (Winsock error " + std::to_string(error) + ").");
		}
#else
		const size_t n = ::send(SID, ptr, size, MSG_NOSIGNAL);
		if (n < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::system_error(errno, std::generic_category(), "send");
		}
#endif
		if (n == 0)
			throw std::runtime_error("Socket send returned zero.");
		ptr += n;
		size -= static_cast<size_t>(n);
	}
}

unsigned int CNetHandlerSocket::recvUInt()
{
	unsigned int netNum = 0;
	readExact(&netNum, sizeof(unsigned int));
	return ntohl(netNum);
}

void CNetHandlerSocket::sendUInt(unsigned int num)
{
	unsigned int netNum = htonl(num);
	writeExact(&netNum, sizeof(unsigned int));
}

int CNetHandlerSocket::recvInt()
{
	unsigned int netNum = 0;
	readExact(&netNum, sizeof(unsigned int));
	return ntohl(netNum);
}

void CNetHandlerSocket::sendInt(int num)
{
	unsigned int netNum = htonl((unsigned int)num);
	writeExact(&netNum, sizeof(unsigned int));
}

unsigned long CNetHandlerSocket::recvUInt64()
{
	unsigned int netHigh = 0;
	unsigned int netLow = 0;
	readExact(&netHigh, sizeof(unsigned int));
	readExact(&netLow, sizeof(unsigned int));

	return (((unsigned long)ntohl(netHigh) & 0xFFFFFFFFLL) << 32) | ((unsigned long)ntohl(netLow));
}

void CNetHandlerSocket::sendUInt64(unsigned long num)
{
	unsigned int netHigh = htonl((unsigned int)(num >> 32));
	unsigned int netLow = htonl((unsigned int)(num & 0xFFFFFFFFLL));
	writeExact(&netHigh, sizeof(unsigned int));
	writeExact(&netLow, sizeof(unsigned int));
}

unsigned char *CNetHandlerSocket::recvDataAlloc(size_t size)
{
	std::unique_ptr<unsigned char[]> data(new unsigned char[size]);
	readExact(data.get(), size);
	return data.release();
}

void CNetHandlerSocket::sendData(unsigned char *data, size_t size)
{
	writeExact(data, size);
}
