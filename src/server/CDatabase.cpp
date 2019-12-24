/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CDatabase.h"
#include "CTransaction.h"
#include "CDatabaseInstance.h"

namespace solusek
{
	IDatabaseInstance *CDatabase::open()
	{
		CDatabaseInstance *di = new CDatabaseInstance(this, ConnectionString);
		return di;
	}

	void CDatabase::closeAll()
	{
		for(std::vector<IDatabaseInstance*>::iterator it = Instances.begin(); it != Instances.end(); ++it)
		{
			delete (*it);
		}
	}

	void CDatabase::setConnectionString(const std::string &cs)
	{
		ConnectionString = cs;
	}

	std::string CDatabase::now()
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time (&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",timeinfo);

		return std::string(buffer);
	}

	void CDatabase::removeInstance(IDatabaseInstance* i)
	{
		for(std::vector<IDatabaseInstance*>::iterator it = Instances.begin(); it != Instances.end(); ++it)
		{
			if((*it) == i)
			{
				Instances.erase(it);
				return;
			}
		}
	}
}
