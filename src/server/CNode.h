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
#include <cstddef>

#define SOLUSEK_DEFAULT_CHUNK_SIZE 4096
#define SOLUSEK_MAX_HEADER_BYTES (32 * 1024)
#define SOLUSEK_MAX_HEADER_COUNT 100
#define SOLUSEK_MAX_BODY_BYTES (16ULL * 1024ULL * 1024ULL)

namespace solusek
{
	class CNode
	{
	private:
		pthread_t ID;
		void *Server;
		CNetHandlerSocket *Socket;
		std::vector<CVarVal*> Headers;
		CLog Log;
		std::string Method, Path, QueryString, ProtocolVersion, Body;
		int ParseErrorStatus;
	public:
		CNode(unsigned int id, void *server, CNetHandlerSocket *socket);
		~CNode();

		pthread_t getID() { return ID; }

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
