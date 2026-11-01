/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifdef USE_MYSQL
#pragma message "Using MySQL"
#include "CDatabaseHandler.h"
#include <stdexcept>

CDatabaseResource::CDatabaseResource(void *resource, const std::string &id)
{
	Resource = resource;
	ID = id;
}

CDatabaseResource::~CDatabaseResource()
{
	if (Resource)
		mysql_free_result((MYSQL_RES *)Resource);
}

CDatabaseTransaction::CDatabaseTransaction(void *obj)
{
	C = static_cast<MYSQL *>(obj);
	Completed = false;
}

CDatabaseTransaction::~CDatabaseTransaction()
{
	if (C && !Completed)
		mysql_rollback(C);
}

void CDatabaseTransaction::commit()
{
	if (!C)
		throw std::runtime_error("Not connected.");
	if (!Completed)
	{
		if (mysql_commit(C) != 0)
			throw std::runtime_error(mysql_error(C));
		Completed = true;
	}
}

CDatabaseHandler::CDatabaseHandler() : C(0)
{
}

CDatabaseHandler::~CDatabaseHandler()
{
	close();
}

bool CDatabaseHandler::open()
{
	if (ConnectionString.size() < 8 || ConnectionString.substr(0, 8) != "mysql://")
		return false;
	size_t n = ConnectionString.find("://");
	if (n == std::string::npos)
		return false;
	std::string cs(ConnectionString.substr(n + 3));
	n = cs.find("/");
	if (n == std::string::npos)
		return false;
	std::string p1(cs.substr(0, n));
	std::string dbname(cs.substr(n + 1));
	std::string username, password, host;
	n = p1.find("@");
	if (n != std::string::npos)
	{
		std::string p2(p1.substr(0, n));
		host = p1.substr(n + 1);
		n = p2.find(":");
		if (n != std::string::npos)
		{
			username = p2.substr(0, n);
			password = p2.substr(n + 1);
		}
		else
			username = p2;
	}
	else
		host = p1;

	C = mysql_init(0);
	if (!C)
		return false;

	if (!mysql_real_connect(C, host.c_str(), username.c_str(), password.c_str(), NULL, 0, NULL, 0))
	{
		enum mysql_protocol_type prot_type = MYSQL_PROTOCOL_TCP;
		mysql_options(C, MYSQL_OPT_PROTOCOL, (void *)&prot_type);
		if (!mysql_real_connect(C, host.c_str(), username.c_str(), password.c_str(), NULL, 0, NULL, 0))
		{
			mysql_close(C);
			C = 0;
			return false;
		}
	}

	if (mysql_select_db(C, dbname.c_str()) != 0)
	{
		mysql_close(C);
		C = 0;
		return false;
	}

	return true;
}

void CDatabaseHandler::close()
{
	if (C)
		mysql_close(C);
	C = 0;
}

CDatabaseTransaction *CDatabaseHandler::begin()
{
	if (!C)
		throw std::runtime_error("Not connected.");
	if (mysql_query(C, "START TRANSACTION") != 0)
		throw std::runtime_error(mysql_error(C));
	return new CDatabaseTransaction(C);
}

int CDatabaseTransaction::exec(const std::string &s)
{
	if (!C)
		throw std::runtime_error("Not connected.");
	return mysql_real_query(C, s.c_str(), s.size());
}

CDatabaseResource *CDatabaseTransaction::insert(const std::string &s, const std::string &idField)
{
	(void)idField;
	if (!C)
		throw std::runtime_error("Not connected.");

	if (mysql_real_query(C, s.c_str(), static_cast<unsigned long>(s.size())) != 0)
	{
		fprintf(stderr, "QUERY ERROR: %s\n", mysql_error(C));
		return new CDatabaseResource(mysql_error(C));
	}

	// INSERT/UPDATE statements normally have no result set. mysql_insert_id()
	// is still valid in that case; do not treat a null MYSQL_RES as failure.
	MYSQL_RES *res = mysql_store_result(C);
	if (!res && mysql_field_count(C) != 0)
		return new CDatabaseResource(mysql_error(C));

	char buf[64];
	snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(mysql_insert_id(C)));
	// CDatabaseResource takes ownership of res (including a null result).
	return new CDatabaseResource(res, buf);
}

bool CDatabaseTransaction::query(const std::string &s, void (*callback)(const DBROW &, void *), void *param)
{
	if (!C)
		throw std::runtime_error("Not connected.");
	if (mysql_real_query(C, s.c_str(), s.size()) == 0)
	{
		MYSQL_RES *res = mysql_store_result(C);
		if (res)
		{
			MYSQL_ROW row;
			unsigned int lengths = mysql_num_fields(res);
			while (row = mysql_fetch_row(res))
			{
				DBROW result;
				for (int n = 0; n < lengths; n++)
				{
					if (row[n])
						result.push_back(row[n]);
					else
						result.push_back("");
				}
				if (callback)
					callback(result, param);
			}
			mysql_free_result(res);
		}
		return true;
	}
	else
		fprintf(stderr, "QUERY ERROR: %s\n", s.c_str());
	return false;
}

CDatabaseResource *CDatabaseTransaction::queryOne(const std::string &s)
{
	if (!C)
		throw std::runtime_error("Not connected.");
	if (mysql_real_query(C, s.c_str(), s.size()) == 0)
	{
		MYSQL_RES *res = mysql_store_result(C);
		if (res)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			if (row)
			{
				DBROW rr;
				unsigned int lengths = mysql_num_fields(res);
				for (int n = 0; n < lengths; n++)
				{
					if (row[n])
						rr.push_back(row[n]);
					else
						rr.push_back("");
				}
				std::vector<DBROW> rows;
				rows.push_back(rr);
				CDatabaseResource *r = new CDatabaseResource(0, row[0]);
				r->setData(rows);
				mysql_free_result(res);
				return r;
			}
			mysql_free_result(res);
		}
	}
	else
		fprintf(stderr, "QUERY ERROR: %s\n", s.c_str());
	return new CDatabaseResource(0);
}

std::string CDatabaseTransaction::esc(const std::string s)
{
	if (!C)
		throw std::runtime_error("Not connected.");
	char *buf = new char[(s.size() * 2) + 1];
	mysql_real_escape_string(C, buf, s.c_str(), s.size());
	std::string r(buf);
	delete[] buf;
	return r;
}

std::string CDatabaseHandler::esc(CDatabaseTransaction *t, const std::string s)
{
	if (!C)
		throw std::runtime_error("Not connected.");
	if (!t)
		throw std::runtime_error("Transaction not allocated.");

	char *buf = new char[(s.size() * 2) + 1];
	mysql_real_escape_string(C, buf, s.c_str(), s.size());
	std::string r(buf);
	delete[] buf;
	return r;
}

#endif
