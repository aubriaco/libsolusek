/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CTransaction.h"
#include "../include/solusek/string_util.h"
#include "CDatabaseHandle.h"

namespace solusek
{
	CTransaction::~CTransaction()
	{
		cleanup();
		delete T;
	}

	MDatabaseHandle *CTransaction::insert(const char *tableName, std::map<std::string, std::string> vars)
	{
		CDatabaseHandler *h = (CDatabaseHandler*)H;
		std::string acc(""), vals("");
		for(std::map<std::string,std::string>::iterator it = vars.begin(); it != vars.end(); ++it)
		{
			if(acc.size() > 0)
				acc += ",";
			if(vals.size() > 0)
				vals += ",";
			acc += "\"";
			acc += it->first;
			acc += "\"";
			vals += "'";
			vals += esc(it->second);
			vals += "'";
		}

		std::string builder("INSERT INTO \"");
		builder += tableName;
		builder += "\" (";
		builder += acc;
		builder += ") VALUES (";
		builder += vals;
		builder += ")";


		MDatabaseHandle *r = new CDatabaseHandle(T->insert(builder, "id"));
		Handles.push_back(r);
		return r;
	}

	bool CTransaction::update(const char *tableName, std::map<std::string, std::string> find, std::map<std::string, std::string> vars)
	{
		CDatabaseHandler *h = (CDatabaseHandler*)H;
		std::string acc(""), vals("");
		for(std::map<std::string,std::string>::iterator it = find.begin(); it != find.end(); ++it)
		{
			if(acc.size() > 0)
				acc += ",";
			acc += "\"";
			acc += it->first;
			acc += "\"='";
			acc += it->second;
			acc += "'";
		}
		for(std::map<std::string,std::string>::iterator it = vars.begin(); it != vars.end(); ++it)
		{
			if(vals.size() > 0)
				vals += ",";
			vals += "\"";
			vals += it->first;
			vals += "\"='";
			vals += esc(it->second);
			vals += "'";
		}

		std::string builder("UPDATE \"");
		builder += tableName;
		builder += "\" SET ";
		builder += vals;
		builder += " WHERE ";
		builder += acc;


		return T->query(builder,0);
	}

	MDatabaseHandle *CTransaction::selectOne(const char *tableName, std::vector<std::string> find, std::map<std::string, std::string> vars, const std::string &sort)
	{
		std::string acc(""), q("");
		for(std::vector<std::string>::iterator it = find.begin(); it != find.end(); ++it)
		{
			if(q.size() > 0)
				q += ",";
			q += "\"";
			q += (*it);
			q += "\"";
		}
		for(std::map<std::string,std::string>::iterator it = vars.begin(); it != vars.end(); ++it)
		{
			if(acc.size() > 0)
				acc += ",";
			acc += "\"";
			acc += it->first;
			acc += "\"='";
			acc += esc(it->second);
			acc += "'";
		}

		std::string builder("SELECT " + q + " FROM \"");
		builder += tableName;
		builder += "\"";
		if(vars.size() > 0)
		{
			builder += " WHERE ";
			builder += acc;
		}
		if(sort.size() > 0)
		{
			builder += " ORDER BY ";
			builder += sort;
		}
		builder += " LIMIT 1";

		MDatabaseHandle *r = new CDatabaseHandle(T->queryOne(builder));
		Handles.push_back(r);
		return r;
	}

	MDatabaseHandle *CTransaction::selectCustom(const std::string &query)
	{
		CDatabaseResource* rs = new CDatabaseResource(0, "");

		T->query(query, selectCallback, rs);

		MDatabaseHandle *r = new CDatabaseHandle(rs);
		Handles.push_back(r);
		return r;
	}

	MDatabaseHandle *CTransaction::select(const char *tableName, std::vector<std::string> find, std::map<std::string, std::string> vars, const std::string &sort, const std::string &limit)
	{
		std::string acc(""), q("");
		for(std::vector<std::string>::iterator it = find.begin(); it != find.end(); ++it)
		{
			if(q.size() > 0)
				q += ",";
			q += "\"";
			q += (*it);
			q += "\"";
		}
		for(std::map<std::string,std::string>::iterator it = vars.begin(); it != vars.end(); ++it)
		{
			if(acc.size() > 0)
				acc += ",";
			acc += "\"";
			acc += it->first;
			acc += "\"='";
			acc += esc(it->second);
			acc += "'";
		}

		std::string builder("SELECT " + q + " FROM \"");
		builder += tableName;
		builder += "\"";
		if(vars.size() > 0)
		{
			builder += " WHERE ";
			builder += acc;
		}
		if(sort.size() > 0)
		{
			builder += " ORDER BY ";
			builder += sort;
		}
		if(limit.size() > 0)
		{
			builder += " LIMIT ";
			builder += limit;
		}


		CDatabaseResource* rs = new CDatabaseResource(0, "");

		T->query(builder, selectCallback, rs);

		MDatabaseHandle *r = new CDatabaseHandle(rs);
		Handles.push_back(r);
		return r;
	}

	void CTransaction::selectCallback(const DBROW &row, void *rs)
	{
		CDatabaseResource *res = (CDatabaseResource*)rs;
		std::vector<std::string> r;
		for(unsigned int n = 0; n < row.size(); n++)
		{
			r.push_back(row[n]);
		}
		res->Data.push_back(r);
	}

	void CTransaction::dispose()
	{
		delete this;
	}

	void CTransaction::commit()
	{
		//T->commit();
		T->exec("commit;");
	}

	void CTransaction::cleanup()
	{
		for(std::vector<MDatabaseHandle*>::iterator it = Handles.begin(); it != Handles.end(); ++it)
		{
			delete (CDatabaseHandle*)(*it);
		}
		Handles.clear();
	}

	std::string CTransaction::esc(const std::string esc)
	{
		return T->esc(esc);
	}
}
