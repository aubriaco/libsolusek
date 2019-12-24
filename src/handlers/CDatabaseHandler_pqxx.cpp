/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifdef USE_PQXX
#pragma message "Using PQXX"
#include "CDatabaseHandler.h"

CDatabaseResource::CDatabaseResource(void *resource, const std::string& id)
{
	Resource = resource;
	ID = id;
}

CDatabaseResource::~CDatabaseResource()
{
}

CDatabaseTransaction::CDatabaseTransaction(void *obj)
{
	W = (pqxx::work*)obj;
}

CDatabaseTransaction::~CDatabaseTransaction()
{
	delete W;
}

void CDatabaseTransaction::commit()
{
	W->commit();
}

CDatabaseHandler::CDatabaseHandler()
{
	C = 0;
}

CDatabaseHandler::~CDatabaseHandler()
{
	close();
}

bool CDatabaseHandler::open()
{
	try
	{
		C = new pqxx::connection(ConnectionString.c_str());
	}
	catch(const std::exception &e)
	{
		return false;
	}
	return C ? true : false;
}

void CDatabaseHandler::close()
{
	delete C;
	C = 0;
}

CDatabaseTransaction *CDatabaseHandler::begin()
{
	pqxx::work *w = new pqxx::work(*C);
	return new CDatabaseTransaction(w);
}

int CDatabaseTransaction::exec(const std::string &s)
{
	try
	{
		pqxx::result rs = W->exec(s);
	}
	catch(const std::exception &e)
	{
		return -1;
	}

	return 0;
}

CDatabaseResource *CDatabaseTransaction::insert(const std::string &s, const std::string &idField)
{
	pqxx::work *w = W;
	try
	{
		pqxx::result rs = w->exec(s + " RETURNING " + idField);
		std::string id = rs[0][0].as<std::string>();
		//w->commit();
		//delete w;
		CDatabaseResource *r = new CDatabaseResource(w, id);
		return r;
	}
	catch(const std::exception &e)
	{
		return new CDatabaseResource(e.what());
	}
}



bool CDatabaseTransaction::query(const std::string &s, void(*callback)(const DBROW &, void *), void *param)
{
	try
	{
		pqxx::result rs = W->exec(s);

		for(auto row: rs)
		{
			DBROW result;
			for(int n = 0; n < row.size(); n++)
			{
				result.push_back(row[n].c_str());
			}
			if(callback)
				callback(result, param);
		}
		return true;
	}
	catch(const std::exception &e)
	{
		return false;
	}
}

CDatabaseResource *CDatabaseTransaction::queryOne(const std::string &s)
{
	try
	{
		pqxx::result rs = W->exec(s);

		for(auto row: rs)
		{
			DBROW result;
			for(int n = 0; n < row.size(); n++)
			{
				result.push_back(row[n].c_str());
			}
			std::vector< std::vector<std::string> > data;
			data.push_back(result);
			CDatabaseResource *r = new CDatabaseResource(0, result[0]);
			r->setData(data);
			return r;
		}
	}
	catch(const std::exception &e)
	{
		return new CDatabaseResource(e.what());
	}
	return new CDatabaseResource(0);
}

std::string CDatabaseTransaction::esc(const std::string s)
{
	return W->esc(s);
}

std::string CDatabaseHandler::esc(CDatabaseTransaction *t, const std::string s)
{
	//return pqxx::escape_binary(s);
	return t->esc(s);
}

#endif
