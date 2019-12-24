#ifndef __I_NET_HANDLER_SOCKET_INCLUDED__
#define __I_NET_HANDLER_SOCKET_INCLUDED__
#include <string>

namespace solusek
{

	struct CertificateInfo
	{
		std::string Subject, Issuer, PublicKey;
	};

	class IPAddressItem
	{
	public:
		std::string Addr, From, To;

		IPAddressItem(const std::string& addr) { Addr = addr; }
		IPAddressItem(const std::string& from, const std::string& to) { From = from; To = to; }
	};



	class INetHandlerSocket
	{
	public:
		virtual bool isReady() = 0;

		virtual std::string getClientIPAddress() = 0;

		virtual void addBlacklist(const IPAddressItem& item) = 0;
		virtual void addWhitelist(const IPAddressItem& item) = 0;

		virtual bool isBlacklisted(const std::string& addr) = 0;
		virtual bool isWhitelisted(const std::string& addr) = 0;

		virtual void clearBlacklist() = 0;
		virtual void clearWhitelist() = 0;

		virtual void setSNIHostName(const std::string& name) = 0;
		virtual std::string getSNIHostName() = 0;

		// SSL
		virtual bool setSSLCertificateMem(void *data) = 0;
		virtual bool setSSLPrivateKeyFileMem(void* data) = 0;
		virtual bool setSSLCertificate(const char* filename) = 0;
		virtual bool setSSLPrivateKeyFile(const char* filename) = 0;
		virtual void setSSLCertificatePassword(const char* password) = 0;
		virtual bool setSSLClientCAFile(const char* filename) = 0;
		virtual void addSSLCertificateToSNI(const char* host, const char* filename, const char* keyfilename = 0, bool mutual = true) = 0;
		virtual bool isSSLValid() = 0;
		virtual void setSSLMutual(bool enabled) = 0;
		virtual void mutexSSL() = 0;
		virtual CertificateInfo getClientCertificate() = 0;
		virtual void* openClientCertificateX509() = 0;
		virtual void closeClientCertificateX509() = 0;

		virtual void setSNIEnabled(bool enabled) = 0;
		virtual void setSNIClientHost(const std::string& host) = 0;
		virtual void cleanupSNI() = 0;

		virtual void setThrowExceptions(bool enabled) = 0;

		virtual bool bind(int port, const char* host = 0) = 0;
		virtual int listen() = 0;
		virtual INetHandlerSocket* accept() = 0;
		virtual int readBuffer(void *buf, int len) = 0;
		virtual bool writeBuffer(const void* buf, int len) = 0;
		virtual bool connect(const char* host, int port) = 0;
		virtual bool close() = 0;

		virtual void setTimeout(int timeout) = 0;
		virtual int getTimeout() = 0;

		virtual bool isConnected() = 0;

		virtual void dispose() = 0;
	};
}

#endif
