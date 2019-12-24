/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#if !defined(USE_PQXX) && !defined(USE_MYSQL)
#include "CDatabaseHandler.h"

CDatabaseResource::CDatabaseResource(void *resource, const std::string& id)
{
}

CDatabaseResource::~CDatabaseResource()
{
}

CDatabaseTransaction::CDatabaseTransaction(void *obj)
{
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
	return false;
}

void CDatabaseHandler::close()
{
}

CDatabaseTransaction *CDatabaseHandler::begin()
{
	return 0;
}

int CDatabaseTransaction::exec(const std::string &s)
{
	return 0;
}

CDatabaseResource *CDatabaseTransaction::insert(const std::string &s, const std::string &idField)
{
	return 0;
}



bool CDatabaseTransaction::query(const std::string &s, void(*callback)(const DBROW &, void *), void *param)
{
	return false;
}

CDatabaseResource *CDatabaseTransaction::queryOne(const std::string &s)
{
	return 0;
}

std::string CDatabaseTransaction::esc(const std::string s)
{
	return std::string();
}

std::string CDatabaseHandler::esc(CDatabaseTransaction *t, const std::string s)
{
	return std::string();
}

#endif
