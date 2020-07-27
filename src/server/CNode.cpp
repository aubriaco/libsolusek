/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CNode.h"
#include "CServer.h"
#include "../include/solusek/MErrorResponse.h"
#include <algorithm>
#include <ctime>
#include <unistd.h>
#include "../include/solusek/string_util.h"

namespace solusek
{
	CNode::CNode(unsigned int id, void *server, CNetHandlerSocket *socket)
	{
		ID = id;
		Server = server;
		Socket = socket;
		Socket->setThrowExceptions(true);
		Socket->setTimeout(5);
		Log.setPrefix(std::string("Node"));
	}

	CNode::~CNode()
	{
		Log.print("Cleaning up node.\n");
		for (std::vector<CVarVal *>::iterator it = Headers.begin(); it != Headers.end(); ++it)
		{
			delete (*it);
		}
		Log.print("Node cleanup complete.\n");
	}

	void CNode::run()
	{
		time_t start = time(0);
		CServer *server = (CServer *)Server;
		try
		{
			readHeader();
			readBody();
		}
		catch (int e)
		{
			Path = "";
			Log.print("Connection unexpectedly closed.\n");
		}

		if (Path.size() > 0)
		{
			std::string host;
			if (CVarVal *vv = getHeader("Host"))
				host = vv->Val;

			MEndpoint *ep;
			if (ep = server->getEndpoint(Path.c_str(), Method.c_str()))
			{
				MRequest request(Body);
				if (CVarVal *vv = getHeader("X-SSL-Client-CN"))
					request.SSLClientCN = vv->Val;

				if (CVarVal *vv = getHeader("Cookie"))
				{
					std::string deccum(vv->Val);
					unsigned int sp;
					std::string ck;
					while (deccum.size() > 0)
					{
						sp = deccum.find(";");
						if (sp != std::string::npos && sp < deccum.size())
						{
							ck = deccum.substr(0, sp);
							deccum = std::string(&deccum[sp + 1], deccum.size() - sp - 1);
						}
						else
						{
							ck = deccum;
							deccum = "";
						}
						trim(ck);

						unsigned int np = ck.find("=");
						if (np != std::string::npos)
						{
							std::string cname(ck.substr(0, np));
							std::string cval(ck.substr(np + 1));
							if (cname == "SolusekSID")
								request.SID = cval;
							request.Cookies.push_back(MCookie(cname, cval));
						}
					}
				}

				request.Host = host;
				MResponse response = ep->Callback(Server, request);

				try
				{
					writeResponse(response);
				}
				catch (int e)
				{
					Log.print("Connection unexpectedly closed.\n");
				}
			}
			else if ((ep = server->getStaticEndpoint(Path.c_str(), Method.c_str(), host.c_str())) != 0)
			{
				MResponse response;
				response.Body = ep->Contents;
				response.Code = 200;
				response.ContentType = ep->ContentType;
				try
				{
					writeResponse(response);
				}
				catch (int e)
				{
					Log.print("Connection unexpectedly closed.\n");
				}
				delete ep;
			}
			else
			{
				Log.print(std::string("Endpoint '") + Path + std::string("' not found.\n"));
				MErrorResponse response(404, "Endpoint not found.");
				try
				{
					writeResponse(response);
				}
				catch (int e)
				{
					Log.print("Connection unexpectedly closed.\n");
				}
			}
		}

		Log.print("Exiting node!\n");
		Socket->close();
		delete Socket;
		Log.print("Exited node.\n");
		time_t diff = time(0) - start;
		char s[128];
		sprintf(s, "Execution time: %d\n", (int)diff);
		Log.print(s);
		dispose();
	}

	void CNode::writeString(const std::string &str)
	{
		Socket->writeBuffer(str.c_str(), str.size());
	}

	void CNode::writeResponse(const MResponse &response)
	{
		Log.print("Sending response...\n");
		char codeBuf[16];
		sprintf(codeBuf, "%i", response.Code);
		std::string codeDesc = response.getCodeDescription();
		std::string sdt;

		if (response.Date.size() == 0)
		{
			time_t now = time(0);
			tm *gmtm = gmtime(&now);
			char *dt = asctime(gmtm);
			sdt = dt;
			sdt = sdt.substr(0, sdt.size() - 1);
		}
		else
			sdt = response.Date;

		writeString("HTTP/1.1 ");
		writeString(codeBuf);
		writeString(" ");
		writeString(codeDesc);
		writeString("\r\n");

		writeString("Date: ");
		writeString(sdt);
		writeString("\r\n");
		writeString("Content-Type: ");
		writeString(response.ContentType);
		writeString("\r\n");

		char lenBuf[32];
		snprintf(lenBuf, 32, "%u", (unsigned int)response.Body.size());

		writeString("Content-Length: ");
		writeString(lenBuf);
		writeString("\r\n");

		writeString("Last-Modified: ");
		writeString(sdt);
		writeString("\r\n");

		if (response.SID.size() > 0)
		{
			writeString("Set-Cookie: ");
			writeString("SolusekSID");
			writeString("=");
			writeString(response.SID);
			writeString("; SameSite=Strict");
			writeString("; Path=/");
			writeString("\r\n");
		}

		for (unsigned int n = 0; n < response.Cookies.size(); n++)
		{
			writeString("Set-Cookie: ");
			writeString(response.Cookies[n].Name);
			writeString("=");
			writeString(response.Cookies[n].Value);
			writeString("; SameSite=Strict");
			if (response.Cookies[n].Path.size() > 0)
			{
				writeString("; Path=");
				writeString(response.Cookies[n].Path);
			}
			writeString("\r\n");
		}

		writeString("Server: Solusek/1.0 (Linux)\r\n");
		writeString("Connection: close\r\n");

		writeString("\r\n");

		writeString(response.Body);

		Log.print("Sent!\n");
	}

	void CNode::readHeader()
	{
		std::string acc("");
		int ex = 0;
		char buf[2];
		while (true)
		{
			int r = Socket->readBuffer(buf, 1);
			buf[1] = 0;
			if (buf[0] == '\r' || buf[0] == 0)
				continue;
			if (buf[0] == '\n')
			{
				CVarVal *vv = new CVarVal();
				int sp = acc.find(':');
				if (sp != std::string::npos)
				{
					vv->Var = acc.substr(0, sp);
					std::transform(vv->Var.begin(), vv->Var.end(), vv->Var.begin(), ::tolower);
					vv->Val = acc.substr(sp + 1);
					trim(vv->Val);
				}
				else
					vv->Var = acc;

				//Log.print(vv->Val + "=" + vv->Var + std::string("\n"));

				Headers.push_back(vv);
				acc = "";
				ex++;
				if (ex == 2)
					break;
				continue;
			}
			acc += buf;
			ex = 0;
		}

		if (Headers.size() > 0)
		{
			int sp = Headers[0]->Var.find(' ');
			Method = Headers[0]->Var.substr(0, sp);
			std::string seg2 = Headers[0]->Var.substr(sp + 1);
			sp = seg2.find(' ');
			Path = seg2.substr(0, sp);
			ProtocolVersion = seg2.substr(sp + 1);
		}
	}

	void CNode::readBody()
	{
		Body = "";

		CVarVal *vv = getHeader("content-length");

		if (vv)
		{
			unsigned int len = atoi(vv->Val.c_str());

			Body.reserve(len);

			int read = 0;
			char buf[BATSD_DEFAULT_CHUNK_SIZE + 1];
			for (unsigned int n = 0; n < len; n += read)
			{
				read = Socket->readBuffer(buf, BATSD_DEFAULT_CHUNK_SIZE);
				buf[read] = 0;
				Body += buf;
			}

			//Log.print(Body + std::string("\n"));
		}
	}

	void CNode::dispose()
	{
		CServer *server = (CServer *)Server;
		server->removeNode(this);
		delete this;
	}

	CVarVal *CNode::getHeader(const char *name)
	{
		std::string tmp = strtolower(name);
		std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
		for (std::vector<CVarVal *>::iterator it = Headers.begin(); it != Headers.end(); ++it)
		{
			if (strtolower((*it)->Var) == tmp)
				return (*it);
		}
		return 0;
	}
} // namespace solusek
