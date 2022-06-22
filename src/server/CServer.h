/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_SERVER_INCLUDED__
#define __C_SERVER_INCLUDED__
#include "../handlers/CNetHandler.h"
#include "../handlers/CThreadHandler.h"
#include "../include/solusek/IServer.h"
#include "../include/solusek/MStaticEndpoint.h"
#include "../include/solusek/MMimeType.h"
#include "CConfig.h"
#include "CLog.h"
#include "CNode.h"
#include "CDatabase.h"
#include "CSession.h"
#include "CCache.h"
#include <vector>

#define SOLUSEK_DEFAULT_THREAD_LIMIT 25

namespace solusek
{
	class CServer : public IServer
	{
	private:
		CLog Log;
	protected:
		CNetHandler NH;
		CThreadHandler TH;
		CNetHandlerSocket *MainSocket;
		CConfig *Config;
		CDatabase *Database;
		bool Secure;
		int ListenPort;
		std::string CertFileName, CertPassword, KeyFileName;
		std::vector<CNode*> Nodes;
		std::vector<MEndpoint*> Endpoints;
		std::vector<MStaticEndpoint*> StaticEndpoints;
    std::vector<std::string> StaticIndexes;
		std::vector<CSession*> Sessions;
		std::vector<MMimeType> MimeTypes;
		bool StopSignal;
		unsigned int ThreadLimit, ThreadCount;
	public:

		CServer();
		virtual ~CServer();

		void addDefaultMimeTypes();

		virtual void registerEndpoint(MEndpoint *ep);

		virtual void registerStaticIndex(const char *name);

		virtual void addStaticDirectory(const char *path, const char *host = 0);

		virtual MEndpoint *getEndpoint(const char *path, const char *method);

		virtual MEndpoint *getStaticEndpoint(const char *path, const char *method, const char *host = 0);

		virtual MEndpoint *getStaticIndex(const char *path, const char *method, const char *host = 0);

		virtual void dispose();

		virtual bool loadConfig(const char *fileName);

		virtual IConfig *getConfig() { return Config; }

		virtual void setSecure(bool on) { Secure = on; }

		virtual void setCertificate(const char *pemFileName, const char *keyFileName, const char *password);

		void static interrupt(int sig);

		virtual EErrorCode run();

		virtual IDatabase *getDatabase();

		virtual IFlagCollection *createFlagCollection();

		void removeNode(CNode *node);

		static THREADFUNC thread_runNode(void *param);

		void runNode(pthread_t id, CNetHandlerSocket *socket);

		virtual void setListenPort(int port) { ListenPort = port; }

		virtual void setInterruptCallback(void(*callback)(int));

		virtual void stop();

		virtual void setThreadLimit(unsigned int limit) { ThreadLimit = limit; }

		virtual ISession *startSession();

		virtual ISession *getSession(const std::string& id);

		virtual void destroySession(const std::string& id);

		virtual MMimeType* getMimeType(const std::string &ext);

		virtual void addMimeType(const MMimeType& type);

		virtual void removeMimeType(const std::string& ext);

		virtual void setCacheConfig(const std::string& host, int port);
	};

}

#endif
