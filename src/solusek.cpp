/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "include/solusek/solusek.h"
#include "server/CServer.h"
#include "client/CClient.h"
#include "handlers/CNetHandler.h"

namespace solusek
{
    IServer* createServer()
    {
        return new CServer();
    }

    IClient* createClient()
    {
        return new CClient();
    }

    INetHandlerSocket* createNetHandlerSocket()
    {
        return new CNetHandlerSocket();
    }
}
