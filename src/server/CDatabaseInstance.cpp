/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CDatabaseInstance.h"
#include "CDatabase.h"
#include "CTransaction.h"

namespace solusek
{
	CDatabaseInstance::CDatabaseInstance(void *d,const std::string &connectionString)
	{
		Used = true;
		T = time(0);
		ConnectionString = connectionString;
		D = d;
		H.setConnectionString(connectionString);
		Connected = H.open();
	}

	CDatabaseInstance::~CDatabaseInstance()
	{
		H.close();
	}

	void CDatabaseInstance::dispose()
	{
		Used = false;
	}

	ITransaction *CDatabaseInstance::begin()
	{
		CDatabaseTransaction *t = H.begin();
		return new CTransaction(&H, t);
	}
}
