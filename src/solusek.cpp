/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "include/solusek/solusek.h"
#include "server/CServer.h"
#include "client/CClient.h"
#include "server/CCache.h"
#include "handlers/CNetHandler.h"

namespace solusek
{
    IServer *createServer()
    {
        return new CServer();
    }

    IClient *createClient()
    {
        return new CClient();
    }

    INetHandlerSocket *createNetHandlerSocket(bool ssl, bool sslMutual)
    {
        return new CNetHandlerSocket(-1, ssl, sslMutual);
    }

    ICache *createCache()
    {
        return new CCache();
    }
}
