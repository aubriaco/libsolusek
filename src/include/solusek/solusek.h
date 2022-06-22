/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __EDTAPI_INCLUDED__
#define __EDTAPI_INCLUDED__
#include "IServer.h"
#include "IClient.h"
#include "ICache.h"
#include "INetHandlerSocket.h"
#include "IFlagCollection.h"
#include "MFlag.h"
#include "MRequest.h"
#include "MResponse.h"
#include "MErrorResponse.h"
#include "ILog.h"

namespace solusek
{
	IServer *createServer();
	IClient *createClient();
	INetHandlerSocket *createNetHandlerSocket();
	ICache *createCache();
}

#endif
