/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_SERVER_INCLUDED__
#define __I_SERVER_INCLUDED__
#include "EErrorCode.h"
#include "MEndpoint.h"
#include "IDatabase.h"
#include "IConfig.h"
#include "IFlagCollection.h"
#include "ISession.h"
#include "MMimeType.h"

namespace solusek
{
	class IServer
	{
	public:
		virtual void dispose() = 0;

		virtual bool loadConfig(const char *fileName) = 0;

		virtual void setSecure(bool on) = 0;

		virtual EErrorCode run() = 0;

		virtual IDatabase *getDatabase() = 0;

		virtual IConfig *getConfig() = 0;

		virtual IFlagCollection *createFlagCollection() = 0;

		virtual void setListenPort(int port) = 0;

		virtual void setCertificate(const char *pemFileName, const char *keyFileName, const char *password) = 0;

		virtual void registerEndpoint(MEndpoint *ep) = 0;

		virtual MEndpoint *getEndpoint(const char *path, const char *method) = 0;

    virtual void addStaticDirectory(const char *path, const char *host = 0) = 0;

    virtual void registerStaticIndex(const char *name) = 0;

		virtual void setInterruptCallback(void(*callback)(int)) = 0;

		virtual void stop() = 0;

		virtual void setThreadLimit(unsigned int limit) = 0;

		virtual ISession *startSession() = 0;

		virtual ISession *getSession(const std::string& id) = 0;

		virtual void destroySession(const std::string& id) = 0;

		virtual void addMimeType(const MMimeType& type) = 0;

		virtual void removeMimeType(const std::string& ext) = 0;
	};
}
#endif
