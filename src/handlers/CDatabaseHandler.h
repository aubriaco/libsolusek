/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_DATABASE_HANDLER_INCLUDED__
#define __C_DATABASE_HANDLER_INCLUDED__
#include <string>
#include <map>
#include <vector>
#ifdef USE_PQXX
#include <pqxx/pqxx>
#elif defined(USE_MYSQL)
#include <mysql/mysql.h>
#endif

typedef std::vector<std::string> DBROW;

class CDatabaseResource
{
public:
	std::string ID, Error;
	void *Resource;

	CDatabaseResource(const std::string &error)
	{
		Resource = 0;
		Error = error;
	}
	CDatabaseResource(void *resource, const std::string& id = "");
	~CDatabaseResource();

	void setData(const std::vector< std::vector<std::string> > data)
	{
		Data = data;
	}


	std::vector< std::vector<std::string> > Data;

	std::vector<std::string> getFirst()
	{
		if(Data.size() > 0)
			return Data[0];
		else
			return std::vector<std::string>();
	}

	unsigned int size() { return Data.size(); }

	bool isError()
	{
		return Error.size() > 0;
	}

};

class CDatabaseTransaction
{
private:
	#ifdef USE_PQXX
	pqxx::work *W;
	#elif defined(USE_MYSQL)
	MYSQL *C;
	#endif
public:
	CDatabaseTransaction(void *obj);
	~CDatabaseTransaction();

	int exec(const std::string &s);
	void commit();
	CDatabaseResource *insert(const std::string &s, const std::string &idField);
	bool query(const std::string &s, void(*callback)(const DBROW &, void*), void* param = 0);
	CDatabaseResource *queryOne(const std::string &s);

	std::string esc(const std::string s);

	void dispose();
};


class CDatabaseHandler
{
private:
	#ifdef USE_PQXX
	pqxx::connection *C;
	#elif defined(USE_MYSQL)
	MYSQL *C;
	#endif

	std::string ConnectionString;
public:
	CDatabaseHandler();
	~CDatabaseHandler();

	void setConnectionString(const std::string &s)
	{
		ConnectionString = s;
	}

	bool open();
	void close();

	CDatabaseTransaction *begin();

	std::string esc(CDatabaseTransaction *t, const std::string s);
};

#endif
