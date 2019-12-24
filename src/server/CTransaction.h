/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_TRANSACTION_INCLUDED__
#define __C_TRANSACTION_INCLUDED__
#include "../handlers/CDatabaseHandler.h"
#include "../include/solusek/ITransaction.h"
#include "../include/solusek/MDatabaseHandle.h"
#include <vector>

namespace solusek
{
	class CTransaction : public ITransaction
	{
	private:
		CDatabaseTransaction *T;
		void *H;

		std::vector<MDatabaseHandle*> Handles;

	public:
		CTransaction(void *h, CDatabaseTransaction *t) : T(t) {}
		~CTransaction();

		virtual void dispose();

		virtual void commit();

		virtual void cleanup();

		virtual MDatabaseHandle *insert(const char *tableName, std::map<std::string, std::string> vars);

		virtual bool update(const char *tableName, std::map<std::string, std::string> find, std::map<std::string, std::string> vars);

		virtual MDatabaseHandle *selectOne(const char *tableName, std::vector<std::string> find, std::map<std::string, std::string> vars, const std::string &sort = std::string());

		virtual MDatabaseHandle *selectCustom(const std::string &query);

		virtual MDatabaseHandle *select(const char *tableName, std::vector<std::string> find, std::map<std::string, std::string> vars, const std::string &sort = std::string(), const std::string &limit = std::string());

		static void selectCallback(const DBROW& row, void *rs);

		std::string esc(const std::string s);
	};
}

#endif
