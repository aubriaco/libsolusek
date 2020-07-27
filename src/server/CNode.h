/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_NODE_INCLUDED__
#define __C_NODE_INCLUDED__
#include "../handlers/CNetHandler.h"
#include "../include/solusek/MEndpoint.h"
#include "CVarVal.h"
#include "CLog.h"
#include <vector>

#define BATSD_DEFAULT_CHUNK_SIZE 1024

namespace solusek
{
	class CNode
	{
	private:
		unsigned int ID;
		void *Server;
		CNetHandlerSocket *Socket;
		std::vector<CVarVal*> Headers;
		CLog Log;
		std::string Method, Path, QueryString, ProtocolVersion, Body;
	public:
		CNode(unsigned int id, void *server, CNetHandlerSocket *socket);
		~CNode();

		int getID() { return ID; }

		void run();

		void readHeader();
		void readBody();

		void writeString(const std::string &str);
		void writeResponse(const MResponse &response);

		void dispose();

		CVarVal *getHeader(const char *name);
	};
}

#endif
