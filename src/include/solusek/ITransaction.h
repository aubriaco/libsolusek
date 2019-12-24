/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_TRANSACTION_INCLUDED__
#define __I_TRANSACTION_INCLUDED__
#include "MDatabaseHandle.h"
#include <string>
#include <vector>
#include <map>

namespace solusek
{
	typedef std::vector<std::string> TArr;
	typedef std::map<std::string, std::string> TDict;

	class ITransaction
	{
	public:
		virtual MDatabaseHandle *insert(const char *tableName, std::map<std::string, std::string> vars) = 0;

		virtual MDatabaseHandle *selectOne(const char *tableName, std::vector<std::string> find, std::map<std::string, std::string> vars, const std::string &sort = std::string()) = 0;

		virtual MDatabaseHandle *select(const char *tableName, std::vector<std::string> find, std::map<std::string, std::string> vars, const std::string &sort = std::string(), const std::string &limit = std::string()) = 0;

		virtual MDatabaseHandle *selectCustom(const std::string &query = std::string()) = 0;

		virtual bool update(const char *tableName, std::map<std::string, std::string> find, std::map<std::string, std::string> vars) = 0;

		virtual void dispose() = 0;

		virtual void commit() = 0;
		virtual void cleanup() = 0;

		virtual std::string esc(const std::string s) = 0;
	};
}

#endif
