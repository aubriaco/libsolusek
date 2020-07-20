/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifdef USE_MYSQL
#pragma message "Using MySQL"
#include "CDatabaseHandler.h"

CDatabaseResource::CDatabaseResource(void *resource, const std::string& id)
{
	Resource = resource;
	ID = id;
}

CDatabaseResource::~CDatabaseResource()
{
	if(Resource)
		mysql_free_result((MYSQL_RES*)Resource);
}

CDatabaseTransaction::CDatabaseTransaction(void *obj)
{
	C = (MYSQL*)obj;
}

CDatabaseTransaction::~CDatabaseTransaction()
{
}

void CDatabaseTransaction::commit()
{
}

CDatabaseHandler::CDatabaseHandler()
{
}

CDatabaseHandler::~CDatabaseHandler()
{
}

bool CDatabaseHandler::open()
{
	if(ConnectionString.size() < 8 || ConnectionString.substr(0, 8) != "mysql://")
		return false;
	size_t n = ConnectionString.find("://");
	if(n == std::string::npos)
		return false;
  	std::string cs(ConnectionString.substr(n+3));
	n = cs.find("/");
	if(n == std::string::npos)
		return false;
	std::string p1(cs.substr(0, n));
	std::string dbname(cs.substr(n+1));
	std::string username, password, host;
  	n = p1.find("@");
	if(n != std::string::npos)
	{
		std::string p2(p1.substr(0, n));
		host = p1.substr(n+1);
		n = p2.find(":");
		if(n != std::string::npos)
		{
			username = p2.substr(0, n);
			password = p2.substr(n+1);
		}
		else
			username = p2;
	}
	else
		host = p1;

	C = mysql_init(0);
	if(!C)
		return false;

  	if(!mysql_real_connect(C, host.c_str(), username.c_str(), password.c_str(), NULL, 0, NULL, 0))
		return false;

	if(mysql_select_db(C, dbname.c_str()) != 0)
		return false;
}

void CDatabaseHandler::close()
{
	mysql_close(C);
}

CDatabaseTransaction *CDatabaseHandler::begin()
{
	return new CDatabaseTransaction(C);
}

int CDatabaseTransaction::exec(const std::string &s)
{
	return mysql_real_query(C, s.c_str(), s.size());
}

CDatabaseResource *CDatabaseTransaction::insert(const std::string &s, const std::string &idField)
{
	if(mysql_real_query(C, s.c_str(), s.size()) == 0)
	{
			MYSQL_RES *res = mysql_store_result(C);
			if(!res)
			{
				unsigned long int iid = mysql_insert_id(C);
				char buf[64];
				sprintf(buf, "%lu", iid);
				CDatabaseResource *r = new CDatabaseResource(res, buf);
				return r;
			}
	}

	return new CDatabaseResource(0);
}



bool CDatabaseTransaction::query(const std::string &s, void(*callback)(const DBROW &, void *), void *param)
{
	if(mysql_real_query(C, s.c_str(), s.size()) == 0)
 	{
 			MYSQL_RES *res = mysql_store_result(C);
 			if(!res)
 			{
				MYSQL_ROW row;
				unsigned long int *lengths = mysql_fetch_lengths(res);
				while(row = mysql_fetch_row(res))
				{
					DBROW result;
					for(int n = 0; n < *lengths; n++)
					{
						if(row[n])
							result.push_back(row[n]);
						else
							result.push_back("");
					}
					if(callback)
				  	callback(result, param);
				}
 				return true;
 			}
 	}
	return false;
}

CDatabaseResource *CDatabaseTransaction::queryOne(const std::string &s)
{
	if(mysql_real_query(C, s.c_str(), s.size()) == 0)
	{
			MYSQL_RES *res = mysql_store_result(C);
			if(!res)
			{
				MYSQL_ROW row = mysql_fetch_row(res);
				if(row)
				{
					DBROW rr;
					unsigned long int *lengths = mysql_fetch_lengths(res);
					for(int n = 0; n < *lengths; n++)
					{
						if(row[n])
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
			}
	}
	return new CDatabaseResource(0);
}

std::string CDatabaseTransaction::esc(const std::string s)
{
	char buf[(s.size()*2)+1];
	mysql_real_escape_string(C, buf, s.c_str(), s.size());
	return std::string(buf);
}

std::string CDatabaseHandler::esc(CDatabaseTransaction *t, const std::string s)
{
	char buf[(s.size()*2)+1];
	mysql_real_escape_string(C, buf, s.c_str(), s.size());
	return std::string(buf);
}

#endif
