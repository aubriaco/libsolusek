/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifdef USE_OPENSSL
#include "CNetHandler.h"
#include <stdlib.h>

using namespace solusek;

static bool bThreadSetup = false;
static pthread_mutex_t *lock_cs;
static long *lock_count;
static int ssl_session_ctx_id = 1;
static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx);

void pthreads_locking_callback(int mode, int type, const char *file, int line)
{
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(lock_cs[type]));
        lock_count[type]++;
    } else {
        pthread_mutex_unlock(&(lock_cs[type]));
    }
}

void pthreads_thread_id(CRYPTO_THREADID *tid)
{
    CRYPTO_THREADID_set_numeric(tid, (unsigned long)pthread_self());
}

void thread_setup(void)
{
    int i;

    lock_cs = (pthread_mutex_t*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
    lock_count = (long int*)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(long));
    for (i = 0; i < CRYPTO_num_locks(); i++) {
        lock_count[i] = 0;
        pthread_mutex_init(&(lock_cs[i]), NULL);
    }

    CRYPTO_THREADID_set_callback(pthreads_thread_id);
    CRYPTO_set_locking_callback(pthreads_locking_callback);
    bThreadSetup = true;
}

void CNetHandlerSocket::mutexSSL()
{
	CRYPTO_THREADID thread_id;

	CRYPTO_THREADID_current(&thread_id);
}


bool CNetHandlerSocket::isConnected()
{
	char buf;
	int err = recv(SID, &buf, 1, MSG_PEEK);
	if(err == SOCKET_ERROR)
		return false;
	else
		return true;
}

void CNetHandlerSocket::setupSSL(bool re)
{
	CNetHandler::initDependencies();

	if(!bThreadSetup)
		thread_setup();

#ifdef LIBSSL1_1
	CTX = SSL_CTX_new(TLS_method());
#else
  CTX = SSL_CTX_new(TLSv1_2_method());
#endif
  SSL_CTX_set_session_id_context(CTX, (const unsigned char *)&ssl_session_ctx_id, sizeof(ssl_session_ctx_id));
  ChildCTX = 0;


	//SSL_CTX_set_options(CTX, SSL_OP_SINGLE_DH_USE);
	if (!CTX)
	{

		ERR_print_errors_fp(stderr);
		return;
	}


	if(re)
	{
		setSSLMutual(SSLMutual);
		setSSLCertificatePassword(CertificatePassword.c_str());
		setSSLCertificate(CertificateFileName.c_str());
		setSSLPrivateKeyFile(PrivateKeyFileName.c_str());
    //setSNIEnabled(SNIEnabled);
		//setSSLClientCAFile(ClientCAFileName.c_str());
	}

}

int CNetHandlerSocket::_sni_callback(SSL *ssl, int *i, void* param)
{
  CNetHandlerSocket *socket = (CNetHandlerSocket*)param;
  printf("SNI Callback called.\n");
  int type = SSL_get_servername_type(ssl);
  const char* cname = SSL_get_servername(ssl, type);
  if(cname == 0)
    return SSL_TLSEXT_ERR_NOACK;
  std::string name(cname);
  printf("SNI Host: %s\n", name.c_str());
  for(std::vector<SNIObj>::iterator it = socket->SNIList.begin(); it != socket->SNIList.end(); ++it)
  {
    if(name == (*it).Host)
    {
      printf("SNI Found Cert for %s\n", name.c_str());
#ifdef LIBSSL1_1
      SSL_CTX *ctx = SSL_CTX_new(TLS_method());
#else
      SSL_CTX *ctx =  SSL_CTX_new(TLSv1_2_method());
#endif
      SSL_CTX_use_certificate(ctx, (X509*)(*it).Cert);
      for(std::vector<void*>::iterator ait = (*it).Aux.begin(); ait != (*it).Aux.end(); ++ait)
      {
        SSL_CTX_add_extra_chain_cert(ctx, (X509*)(*ait));
      }
      SSL_CTX_use_PrivateKey(ctx, (EVP_PKEY*)(*it).Key);
      SSL_set_SSL_CTX(ssl, ctx);
      socket->setChildCTX(ctx);
      socket->setSNIHostName(name);
      if((*it).Mutual)
    	{
    		SSL_CTX_set_verify(ctx,SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,verify_callback);
    		SSL_CTX_set_verify_depth(ctx,1);
    	}
    	else
    	{
    		SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    		SSL_CTX_set_verify_depth(ctx,0);
    	}
      return SSL_TLSEXT_ERR_OK;
    }
  }
  return SSL_TLSEXT_ERR_NOACK;
}

void CNetHandlerSocket::setCTX(void *ctx)
{
  CTX = (SSL_CTX*)ctx;
}

void CNetHandlerSocket::setChildCTX(void *ctx)
{
  ChildCTX = (SSL_CTX*)ctx;
}

void CNetHandlerSocket::setSNIEnabled(bool enabled)
{
    SNIEnabled = enabled;
    if(enabled)
    {
      SSL_CTX_set_tlsext_servername_callback(CTX, _sni_callback);
      SSL_CTX_set_tlsext_servername_arg(CTX, this);
    }
    else
    {
      SSL_CTX_set_tlsext_servername_callback(CTX, 0);
      SSL_CTX_set_tlsext_servername_arg(CTX, 0);
    }
}

void CNetHandlerSocket::setSNIClientHost(const std::string& host)
{
  SSL_set_tlsext_host_name(_SSL, host.c_str());
}

void CNetHandlerSocket::setupSSLSocket(int sid, bool accept, const char* host)
{
	int err = 0;
	_SSL = SSL_new(CTX);
	err = SSL_set_fd(_SSL, sid);

	if(err == -1)
		ERR_print_errors_fp(stderr);

	if(accept)
		err = SSL_accept(_SSL);
	else
  {
    SSL_set_connect_state(_SSL);
    if(host)
      setSNIClientHost(host);
		err = SSL_connect(_SSL);
  }
	if (err == -1)
		ERR_print_errors_fp(stderr);
}

bool CNetHandlerSocket::setSSLCertificate(const char* filename)
{
	CertificateFileName = filename;
	if(SSL_CTX_use_certificate_file(CTX, filename, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	return true;
}

bool CNetHandlerSocket::setSSLCertificateMem(void *data)
{
	if(SSL_CTX_use_certificate(CTX, (X509*)data) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	return true;
}

const char* passwd;

int pem_passwd_cb(char *buf, int size, int rwflag, void *password)
{
	strncpy(buf, (char *)(passwd), size);
	buf[size - 1] = '\0';
	return(strlen(buf));
}


void CNetHandlerSocket::setSSLCertificatePassword(const char* password)
{
	passwd = password;
	CertificatePassword = password;
	::SSL_CTX_set_default_passwd_cb(CTX, pem_passwd_cb);
}

bool CNetHandlerSocket::setSSLPrivateKeyFile(const char* filename)
{
	PrivateKeyFileName = filename;
	if(SSL_CTX_use_PrivateKey_file(CTX, filename, SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	return true;
}

bool CNetHandlerSocket::setSSLPrivateKeyFileMem(void* data)
{
	if(SSL_CTX_use_PrivateKey(CTX, (EVP_PKEY*)data) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	return true;
}

bool CNetHandlerSocket::setSSLClientCAFile(const char* filename)
{
	ClientCAFileName = filename;
	if (SSL_CTX_load_verify_locations(CTX, filename, NULL) != 1)
    {
        ERR_print_errors_fp(stderr);
		return false;
    }

    // allow this CA to be sent to the client during handshake

    STACK_OF(X509_NAME) * list = SSL_load_client_CA_file(filename);
    if (NULL == list)
    {
        printf("Failed to load SSL client CA file.\n");
		return false;
    }
    SSL_CTX_set_client_CA_list(CTX, list);
    SSL_CTX_set_verify_depth(CTX, 1);

	return true;
}

bool CNetHandlerSocket::isSSLValid()
{
	return SSL_CTX_check_private_key(CTX);
}

static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	return 1;
}

void CNetHandlerSocket::setSSLMutual(bool enabled)
{
	SSLMutual = enabled;
	if(enabled)
	{
		SSL_CTX_set_verify(CTX,SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE,verify_callback);
		SSL_CTX_set_verify_depth(CTX,1);
	}
	else
	{
		SSL_CTX_set_verify(CTX, SSL_VERIFY_NONE, NULL);
		SSL_CTX_set_verify_depth(CTX,0);
	}
}

void* CNetHandlerSocket::openClientCertificateX509()
{
	ClientCertificate = SSL_get_peer_certificate(_SSL);
	return ClientCertificate;
}

void CNetHandlerSocket::closeClientCertificateX509()
{
	if(ClientCertificate)
		X509_free(ClientCertificate);
}

CertificateInfo CNetHandlerSocket::getClientCertificate()
{
	CertificateInfo info;
	if(SSLMutual)
	{
		char* str;
		ClientCertificate = SSL_get_peer_certificate(_SSL);
		if(ClientCertificate)
		{
			str = X509_NAME_oneline(X509_get_subject_name(ClientCertificate), 0, 0);
			info.Subject = str;
			free(str);
			str = X509_NAME_oneline(X509_get_issuer_name(ClientCertificate), 0, 0);
			info.Issuer = str;
			free(str);
			info.PublicKey = getSSLClientCertificatePublicKey();

			X509_free(ClientCertificate);
		}
	}
	return info;
}

std::string uc2hexstr(unsigned char* buf, const unsigned int len)
{
	std::string ret;
	for(unsigned int i = 0; i < len; i++)
	{
		char str[16];
		sprintf(str,"%02x",buf[i]);
		ret += str;
	}
	return ret;
}

void CNetHandlerSocket::addSSLCertificateToSNI(const char* host, const char* filename, const char* keyfilename, bool mutual)
{
  std::vector<void*> aux;
  FILE* file = fopen(filename, "rb");
  if(!file)
  {
    printf("Cert not found: %s\n", filename);
    return;
  }
  X509 *cert = PEM_read_X509(file, 0, 0, 0);
  if(!cert)
  {
    printf("Invalid cert: %s\n", filename);
    return;
  }
  while(!feof(file))
  {
    X509* acert = PEM_read_X509(file,0,0,0);
    if(acert)
      aux.push_back(acert);
  }
  fclose(file);
  file = fopen(keyfilename ? keyfilename : filename, "rb");
  if(!file)
  {
    printf("Key not found: %s\n", keyfilename ? keyfilename : filename);
    return;
  }
  EVP_PKEY *key = PEM_read_PrivateKey(file, 0, 0, 0);
  fclose(file);
  if(!key)
  {
    printf("Invalid key.\n");
    X509_free(cert);
    return;
  }
  SNIObj obj;
  obj.Host = host;
  obj.Cert = cert;
  obj.Key = key;
  obj.Aux = aux;
  obj.Mutual = mutual;
  SNIList.push_back(obj);
}

void CNetHandlerSocket::cleanupSNI()
{
  for(std::vector<SNIObj>::iterator it = SNIList.begin(); it != SNIList.end(); ++it)
  {
    X509_free((X509*)(*it).Cert);
    EVP_PKEY_free((EVP_PKEY*)(*it).Key);
    for(std::vector<void*>::iterator ait = (*it).Aux.begin(); ait != (*it).Aux.end(); ++ait)
    {
      X509_free((X509*)(*ait));
    }
  }
  SNIList.empty();
}

std::string CNetHandlerSocket::getSSLClientCertificatePublicKey()
{
	std::string ret;
	EVP_PKEY * pubkey = X509_get_pubkey(ClientCertificate);

	if(pubkey)
	{

		unsigned char *ucBuf, *uctempBuf;

		const int len = i2d_PublicKey(pubkey, 0);

		ucBuf = new unsigned char[len+1];
		uctempBuf = ucBuf;


		i2d_PublicKey(pubkey,&uctempBuf);

		ret = uc2hexstr(ucBuf, len);

		delete[] ucBuf;

		EVP_PKEY_free(pubkey);


	}
	else
		printf("Failed to get public key.\n");


	return ret;
}

void CNetHandlerSocket::cleanupSSL()
{
  cleanupSNI();
  if(_SSL)
	 SSL_free(_SSL);
  if(CTXRoot && CTX)
	 SSL_CTX_free(CTX);
  _SSL = 0;
  CTX = 0;
}

#endif
