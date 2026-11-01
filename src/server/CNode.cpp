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
#include <limits>
#include <cstdint>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cctype>
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
		ParseErrorStatus = 0;
	}

	CNode::~CNode()
	{
		if(Socket)
			delete Socket;
		Log.print("Cleaning up node.\n");
		for (std::vector<CVarVal *>::iterator it = Headers.begin(); it != Headers.end(); ++it)
		{
			if((*it))
				delete (*it);
		}
		Log.print("Node cleanup complete.\n");
	}

	void CNode::run()
	{
		time_t start = time(0); CServer *server = static_cast<CServer *>(Server);
		try { readHeader(); readBody(); }
		catch (const std::length_error &e) { Path.clear(); ParseErrorStatus = 413; Log.print(std::string("Request rejected: ") + e.what() + "\n"); }
		catch (const std::exception &e) { Path.clear(); ParseErrorStatus = 400; Log.print(std::string("Malformed request: ") + e.what() + "\n"); }
		catch (...) { Path.clear(); ParseErrorStatus = 400; }
		if (!Path.empty())
		{
			std::string host; if (CVarVal *h = getHeader("Host")) host = h->Val;
			MEndpoint *ep = server->getEndpoint(Path.c_str(), Method.c_str());
			if (ep)
			{
				MRequest request(Body); request.QueryString = QueryString; request.Host = host;
				if (CVarVal *h = getHeader("X-SSL-Client-CN")) request.SSLClientCN = h->Val;
				if (CVarVal *h = getHeader("Cookie")) { std::string all = h->Val; std::size_t pos = 0; while (pos < all.size()) { std::size_t e = all.find(';', pos); std::string item = all.substr(pos, e == std::string::npos ? std::string::npos : e-pos); trim(item); std::size_t eq = item.find('='); if (eq != std::string::npos) { std::string name=item.substr(0,eq), value=item.substr(eq+1); if(name=="SolusekSID") request.SID=value; request.Cookies.push_back(MCookie(name,value)); } if(e==std::string::npos) break; pos=e+1; } }
				try { writeResponse(ep->Callback(Server, request)); } catch (...) { Log.print("Failed to write endpoint response.\n"); }
			}
			else if ((ep = server->getStaticEndpoint(Path.c_str(), Method.c_str(), host.c_str())) != 0)
			{
				MResponse response; response.Body=ep->Contents; response.Code=200; response.ContentType=ep->ContentType; try { writeResponse(response); } catch (...) {} delete ep;
			}
			else { MErrorResponse response(404, "Endpoint not found."); try { writeResponse(response); } catch (...) {} }
		}
		else if (Socket && ParseErrorStatus)
		{
			MResponse response; response.Code=ParseErrorStatus; response.ContentType="text/plain; charset=utf-8"; response.Body=ParseErrorStatus==413 ? "Request body too large." : "Bad request."; try { writeResponse(response); } catch (...) {}
		}
		if (Socket) { Socket->close(); delete Socket; Socket=0; }
		char elapsed[96]; snprintf(elapsed,sizeof(elapsed),"Execution time: %ld\n",(long)(time(0)-start)); Log.print(elapsed);
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
			struct tm utcTime;
			char dateBuffer[64];
			if (gmtime_r(&now, &utcTime) && strftime(dateBuffer, sizeof(dateBuffer), "%a, %d %b %Y %H:%M:%S GMT", &utcTime))
				sdt = dateBuffer;
			else
				sdt = "Thu, 01 Jan 1970 00:00:00 GMT";
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
		snprintf(lenBuf, sizeof(lenBuf), "%zu", response.Body.size());

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

	static bool isTokenChar(unsigned char c)
	{
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) return true;
		return std::strchr("!#$%&'*+-.^_`|~", c) != 0;
	}

	static std::string readHttpLine(CNetHandlerSocket *socket, std::size_t &total)
	{
		std::string line;
		char c = 0;
		while (true)
		{
			if (socket->readBuffer(&c, 1) != 1) throw std::runtime_error("Unexpected end of request");
			if (++total > SOLUSEK_MAX_HEADER_BYTES) throw std::length_error("HTTP headers exceed configured limit");
			if (c == '\n') { if (!line.empty() && line.back() == '\r') line.pop_back(); return line; }
			if (c == '\0') throw std::runtime_error("NUL byte in HTTP headers");
			if (c != '\r') line += c;
			if (line.size() > 8192) throw std::length_error("HTTP line exceeds configured limit");
		}
	}

	void CNode::readHeader()
	{
		for (std::size_t i = 0; i < Headers.size(); ++i) delete Headers[i];
		Headers.clear();
		std::size_t total = 0;
		std::string requestLine = readHttpLine(Socket, total);
		std::size_t a = requestLine.find(' '), b = a == std::string::npos ? std::string::npos : requestLine.find(' ', a + 1);
		if (a == std::string::npos || b == std::string::npos || a == 0 || b == a + 1) throw std::runtime_error("Invalid request line");
		Method = requestLine.substr(0, a);
		for (std::size_t i = 0; i < Method.size(); ++i) if (!isTokenChar((unsigned char)Method[i])) throw std::runtime_error("Invalid HTTP method");
		Path = requestLine.substr(a + 1, b - a - 1);
		ProtocolVersion = requestLine.substr(b + 1);
		if (ProtocolVersion != "HTTP/1.0" && ProtocolVersion != "HTTP/1.1") throw std::runtime_error("Unsupported HTTP version");
		if (Path.empty() || Path[0] != '/') throw std::runtime_error("Invalid request target");
		std::size_t q = Path.find('?');
		if (q != std::string::npos) { QueryString = Path.substr(q + 1); Path.resize(q); }

		for (std::size_t count = 0; count < SOLUSEK_MAX_HEADER_COUNT; ++count)
		{
			std::string line = readHttpLine(Socket, total);
			if (line.empty()) return;
			if (line[0] == ' ' || line[0] == '\t') throw std::runtime_error("Folded HTTP headers are not accepted");
			std::size_t colon = line.find(':');
			if (colon == std::string::npos || colon == 0) throw std::runtime_error("Malformed HTTP header");
			CVarVal *h = new CVarVal();
			h->Var = line.substr(0, colon);
			for (std::size_t i = 0; i < h->Var.size(); ++i) if (!isTokenChar((unsigned char)h->Var[i])) { delete h; throw std::runtime_error("Invalid HTTP header name"); }
			std::transform(h->Var.begin(), h->Var.end(), h->Var.begin(), [](unsigned char c) { return (char)std::tolower(c); });
			h->Val = line.substr(colon + 1); trim(h->Val);
			for (std::size_t i = 0; i < h->Val.size(); ++i) if ((unsigned char)h->Val[i] < 0x20 && h->Val[i] != '\t') { delete h; throw std::runtime_error("Invalid HTTP header value"); }
			Headers.push_back(h);
		}
		throw std::length_error("Too many HTTP headers");
	}

	void CNode::readBody()
	{
		Body.clear();
		CVarVal *cl = getHeader("content-length"), *te = getHeader("transfer-encoding");
		unsigned int contentLengthCount = 0, transferEncodingCount = 0, hostCount = 0;
		for (std::size_t i = 0; i < Headers.size(); ++i)
		{
			if (Headers[i]->Var == "content-length") ++contentLengthCount;
			if (Headers[i]->Var == "transfer-encoding") ++transferEncodingCount;
			if (Headers[i]->Var == "host") ++hostCount;
		}
		if (contentLengthCount > 1 || transferEncodingCount > 1 || hostCount > 1)
			throw std::runtime_error("Duplicate HTTP message framing header");
		if (ProtocolVersion == "HTTP/1.1" && !getHeader("host"))
			throw std::runtime_error("HTTP/1.1 Host header is required");
		if (cl && te) throw std::runtime_error("Content-Length and Transfer-Encoding cannot be combined");
		if (te)
		{
			std::string enc = strtolower(te->Val); trim(enc);
			if (enc != "chunked") throw std::runtime_error("Unsupported Transfer-Encoding");
			std::size_t chunkFramingBytes = 0;
			unsigned int chunkCount = 0;
			while (true)
			{
				if (++chunkCount > 65536) throw std::length_error("Too many HTTP chunks");
				std::string sizeLine = readHttpLine(Socket, chunkFramingBytes);
				std::size_t semi = sizeLine.find(';'); std::string hex = sizeLine.substr(0, semi);
				if (hex.empty()) throw std::runtime_error("Invalid chunk size");
				for (std::size_t i = 0; i < hex.size(); ++i)
					if (!std::isxdigit(static_cast<unsigned char>(hex[i]))) throw std::runtime_error("Invalid chunk size");
				char *end = 0; unsigned long long n = std::strtoull(hex.c_str(), &end, 16);
				if (!end || *end || (hex[0] == '-' || hex[0] == '+')) throw std::runtime_error("Invalid chunk size");
				if (!n)
				{
					for (unsigned int i = 0; i < SOLUSEK_MAX_HEADER_COUNT; ++i) if (readHttpLine(Socket, chunkFramingBytes).empty()) return;
					throw std::runtime_error("Too many chunk trailers");
				}
				if (n > SOLUSEK_MAX_BODY_BYTES - Body.size()) throw std::length_error("Request body exceeds configured limit");
				std::size_t left = (std::size_t)n; char buf[SOLUSEK_DEFAULT_CHUNK_SIZE];
				while (left) { int want = (int)std::min<std::size_t>(left, sizeof(buf)); int got = Socket->readBuffer(buf, want); if (got <= 0) throw std::runtime_error("Incomplete chunk data"); Body.append(buf, (std::size_t)got); left -= (std::size_t)got; }
				char crlf[2]; if (Socket->readBuffer(crlf, 2) != 2 || crlf[0] != '\r' || crlf[1] != '\n') throw std::runtime_error("Invalid chunk terminator");
			}
		}
		if (!cl) return;
		if (cl->Val.empty()) throw std::runtime_error("Empty Content-Length");
		unsigned long long length = 0;
		for (std::size_t i = 0; i < cl->Val.size(); ++i) { if (cl->Val[i] < '0' || cl->Val[i] > '9') throw std::runtime_error("Invalid Content-Length"); unsigned int d = cl->Val[i] - '0'; if (length > (std::numeric_limits<unsigned long long>::max() - d) / 10) throw std::length_error("Content-Length overflow"); length = length * 10 + d; }
		if (length > SOLUSEK_MAX_BODY_BYTES) throw std::length_error("Request body exceeds configured limit");
		Body.reserve((std::size_t)length);
		char buf[SOLUSEK_DEFAULT_CHUNK_SIZE]; std::size_t left = (std::size_t)length;
		while (left) { int want = (int)std::min<std::size_t>(left, sizeof(buf)); int got = Socket->readBuffer(buf, want); if (got <= 0) throw std::runtime_error("Incomplete request body"); Body.append(buf, (std::size_t)got); left -= (std::size_t)got; }
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
