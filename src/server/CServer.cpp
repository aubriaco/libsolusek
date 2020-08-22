/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CServer.h"
#include "CTrustStore.h"
#include "CFlagCollection.h"
#include "../include/solusek/string_util.h"
#include "../include/solusek/file_util.h"
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

namespace solusek
{
	struct SServerPackage
	{
		CServer *Server;
		CNetHandlerSocket *Socket;
		unsigned int ID;
	};

	std::string getFileModTime(const std::string &path)
	{
		struct stat attr;
		stat(path.c_str(), &attr);
		return ctime(&attr.st_mtime);
	}

	CServer::CServer()
	{
		StopSignal = false;
		Config = 0;
		CNetHandler::initDependencies();
		Database = new CDatabase();
		MainSocket = 0;
		ListenPort = 80;
		ThreadLimit = EDTAPI_DEFAULT_THREAD_LIMIT;
		ThreadCount = 0;
		addDefaultMimeTypes();
	}

	CServer::~CServer()
	{
		if (Config)
			delete Config;
		for (std::vector<CNode *>::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
			delete (*it);
		for (std::vector<MEndpoint *>::iterator it = Endpoints.begin(); it != Endpoints.end(); ++it)
			delete (*it);
		for (std::vector<CSession *>::iterator it = Sessions.begin(); it != Sessions.end(); ++it)
			delete (*it);
		for (std::vector<MStaticEndpoint *>::iterator it = StaticEndpoints.begin(); it != StaticEndpoints.end(); ++it)
			delete (*it);
		if (Database)
			delete Database;
#ifdef USE_OPENSSL
		//FIPS_mode_set(0);
		EVP_cleanup();
		CRYPTO_cleanup_all_ex_data();
		ERR_free_strings();
		OPENSSL_cleanup();
#endif
	}

	void CServer::addDefaultMimeTypes()
	{
		addMimeType(MMimeType("html", true));
		addMimeType(MMimeType("htm", true));
		addMimeType(MMimeType("txt", true));
		addMimeType(MMimeType("js", true, "text/javascript"));
		addMimeType(MMimeType("css", true));
		addMimeType(MMimeType("png", false, "image/png"));
		addMimeType(MMimeType("jpg", false, "image/jpeg"));
		addMimeType(MMimeType("jpeg", false, "image/jpeg"));
		addMimeType(MMimeType("gif", false, "image/gif"));
		addMimeType(MMimeType("bmp", false, "image/bmp"));
		addMimeType(MMimeType("svg", false, "image/svg"));
	}

	void CServer::dispose()
	{
		delete this;
	}

	IDatabase *CServer::getDatabase()
	{
		return Database;
	}

	IFlagCollection *CServer::createFlagCollection()
	{
		return (IFlagCollection *)new CFlagCollection();
	}

	bool CServer::loadConfig(const char *fileName)
	{
		Config = new CConfig(fileName);
		if (!Config->isOK())
		{
			delete Config;
			Config = 0;
			return false;
		}
		return true;
	}

	void CServer::interrupt(int sig)
	{
	}

	EErrorCode CServer::run()
	{
		NH.initDependencies();

		MainSocket = NH.createSocket(Secure);

		if (!MainSocket->isReady())
		{
			Log.print("Socket not ready.\n");
		}

		if (Secure)
		{
			MainSocket->mutexSSL();
			MainSocket->setSSLMutual(false);
			MainSocket->setSSLCertificatePassword(CertPassword.c_str());
			MainSocket->setSSLCertificate(CertFileName.c_str());
			MainSocket->setSSLPrivateKeyFile(KeyFileName.c_str());
		}

		MainSocket->setTimeout(5);

		if (MainSocket->bind(ListenPort))
		{

			Log.print("Listener initialized.\n");

			while (!StopSignal)
			{

				while (ThreadCount > ThreadLimit)
				{
					sleep(1);
					continue;
				}

				MainSocket->listen();

				CNetHandlerSocket *socket = MainSocket->accept();

				ThreadCount++;

				char tlimit[128];
				sprintf(tlimit, "Thread Count %u of %u allowed.\n", ThreadCount, ThreadLimit);
				Log.print(tlimit);

				if (StopSignal)
				{
					socket->close();
					delete socket;
					break;
				}

				Log.print("Client connection attempt.\n");

				SServerPackage *pkg = new SServerPackage();
				pkg->Server = this;
				pkg->Socket = socket;
				pkg->ID = 0;
				THREADID threadId = 0;

				TH.create(thread_runNode, (void *)pkg, &threadId, false, true);
				pkg->ID = threadId;
			}

			MainSocket->close();
			delete MainSocket;

			if (Nodes.size() > 0)
				Log.print("Waiting for nodes to gracefully terminate.\n");
			while (Nodes.size() > 0)
				sleep(1);

			return E_ERROR_NONE;
		}
		else
		{
			delete MainSocket;
			Log.print("ERROR: Could not bind.\n");
			return E_ERROR_UNKNOWN;
		}
	}

	THREADFUNC CServer::thread_runNode(void *param)
	{
		SServerPackage *pkg = (SServerPackage *)param;
		while (pkg->ID == 0)
			sleep(1);
		pkg->Server->runNode(pkg->ID, pkg->Socket);
		delete pkg;
		pthread_exit(0);
		return 0;
	}

	void CServer::runNode(int id, CNetHandlerSocket *socket)
	{
		Log.print("Running node...\n");
		CNode *node = new CNode(id, this, socket);
		Nodes.push_back(node);
		node->run();
		ThreadCount--;
	}

	void CServer::removeNode(CNode *node)
	{
		for (std::vector<CNode *>::iterator it = Nodes.begin(); it != Nodes.end(); ++it)
		{
			if ((*it)->getID() == node->getID())
			{
				Nodes.erase(it);
				return;
			}
		}
	}

	void CServer::setCertificate(const char *pemFileName, const char *keyFileName, const char *password)
	{
		CertFileName = pemFileName;
		CertPassword = password;
		KeyFileName = keyFileName;
	}

	void CServer::registerEndpoint(MEndpoint *ep)
	{
		Endpoints.push_back(ep);
	}

	void CServer::registerStaticIndex(const char *name)
	{
		StaticIndexes.push_back(name);
	}

	void CServer::addStaticDirectory(const char *path, const char *host)
	{
		MStaticEndpoint *ep = new MStaticEndpoint();
		if (host)
			ep->Host = strtolower(host);
		ep->Path = path;
		StaticEndpoints.push_back(ep);
	}

	MEndpoint *CServer::getEndpoint(const char *path, const char *method)
	{
		for (std::vector<MEndpoint *>::iterator it = Endpoints.begin(); it != Endpoints.end(); ++it)
		{
			if ((*it)->Path == path && (*it)->Method == method)
				return (*it);
		}
		return 0;
	}

	MEndpoint *CServer::getStaticEndpoint(const char *path, const char *method, const char *host)
	{
		std::string fpath(path);
		if (fpath.find("..") != std::string::npos)
			return 0;
		if (fpath[fpath.size() - 1] == '/')
			return getStaticIndex(fpath.c_str(), method, host);

		std::string staticPath, shost;

		if (host)
			shost = strtolower(host);

		for (std::vector<MStaticEndpoint *>::iterator it = StaticEndpoints.begin(); it != StaticEndpoints.end(); ++it)
		{
			if ((*it)->Host.size() == 0)
			{
				staticPath = (*it)->Path;
				break;
			}
			else if ((*it)->Host.size() <= shost.size() && shost.substr(shost.size() - (*it)->Host.size()) == (*it)->Host)
			{
				staticPath = (*it)->Path;
				break;
			}
		}

		if (staticPath.size() == 0)
			return 0;

		MEndpoint *endpoint = 0;

		std::string qs;
		unsigned int np = fpath.find("?");
		if (np != std::string::npos)
		{
			qs = fpath.substr(np + 1);
			fpath = fpath.substr(0, np);
		}

		std::string dpath(staticPath);
		dpath += "/";
		dpath += fpath;

		np = fpath.find_last_of("/");
		if (np == std::string::npos)
			return 0;

		std::string fname(fpath.substr(np + 1));

		std::string ext;
		np = fname.find_last_of(".");

		if (np != std::string::npos)
		{
			ext = strtolower(fname.substr(np + 1));

			MMimeType *mime = getMimeType(ext);

			if (mime)
			{
				if (mime->IsText)
				{
					FILE *file = fopen(dpath.c_str(), "r");
					if (!file)
						return 0;
					fclose(file);
					endpoint = new MEndpoint();
					endpoint->Date = getFileModTime(dpath);
					endpoint->Path = dpath;
					endpoint->Method = method;
					endpoint->Ext = ext;
					endpoint->Contents = readTextFile(dpath);
					endpoint->Type = 1;
					endpoint->ContentType = mime->Type;
				}
				else
				{
					FILE *file = fopen(dpath.c_str(), "rb");
					if (!file)
						return 0;
					fclose(file);
					endpoint = new MEndpoint();
					endpoint->Date = getFileModTime(dpath);
					endpoint->Path = dpath;
					endpoint->Method = method;
					endpoint->Ext = ext;
					std::vector<unsigned char> data = readBinaryFile(dpath);
					endpoint->Contents = std::string((const char *)&data[0], data.size());
					endpoint->Type = 1;
					endpoint->ContentType = mime->Type;
				}
			}
		}
		return endpoint;
	}

	MEndpoint *CServer::getStaticIndex(const char *path, const char *method, const char *host)
	{
		std::string dpath(path);
		for (std::vector<std::string>::iterator it = StaticIndexes.begin(); it != StaticIndexes.end(); ++it)
		{
			if ((*it)[(*it).size() - 1] == '/')
				continue;
			MEndpoint *endpoint = getStaticEndpoint((dpath + (*it)).c_str(), method, host);
			if (endpoint != 0)
			{
				return endpoint;
			}
		}
		return 0;
	}

	void CServer::setInterruptCallback(void (*callback)(int))
	{
		struct sigaction sigIntHandler;

		sigIntHandler.sa_handler = callback;
		sigemptyset(&sigIntHandler.sa_mask);
		sigIntHandler.sa_flags = 0;

		sigaction(SIGINT, &sigIntHandler, NULL);
	}

	void CServer::stop()
	{
		StopSignal = true;
		CNetHandlerSocket *socket = NH.createSocket(Secure);
		socket->connect("localhost", ListenPort);
		socket->close();
		delete socket;
	}

	ISession *CServer::startSession()
	{
		CSession *ses = new CSession();
		Sessions.push_back(ses);
		return ses;
	}

	ISession *CServer::getSession(const std::string &id)
	{
		for (std::vector<CSession *>::iterator it = Sessions.begin(); it != Sessions.end(); ++it)
		{
			if ((*it)->getID() == id)
				return (*it);
		}
		return 0;
	}

	void CServer::destroySession(const std::string &id)
	{
		for (std::vector<CSession *>::iterator it = Sessions.begin(); it != Sessions.end(); ++it)
		{
			if ((*it)->getID() == id)
			{
				Sessions.erase(it);
				return;
			}
		}
	}

	MMimeType *CServer::getMimeType(const std::string &ext)
	{
		for (std::vector<MMimeType>::iterator it = MimeTypes.begin(); it != MimeTypes.end(); ++it)
		{
			if ((*it).Ext == ext)
				return &(*it);
		}
		return 0;
	}

	void CServer::addMimeType(const MMimeType &type)
	{
		MimeTypes.push_back(type);
	}

	void CServer::removeMimeType(const std::string &ext)
	{
		for (std::vector<MMimeType>::iterator it = MimeTypes.begin(); it != MimeTypes.end(); ++it)
		{
			if ((*it).Ext == ext)
			{
				MimeTypes.erase(it);
				return;
			}
		}
	}
} // namespace solusek
