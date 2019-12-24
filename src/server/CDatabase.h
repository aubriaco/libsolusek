/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_DATABASE_INCLUDED__
#define __C_DATABASE_INCLUDED__
#include "../handlers/CDatabaseHandler.h"
#include "../include/solusek/IDatabase.h"
#include "../include/solusek/IDatabaseInstance.h"

namespace solusek
{
	class CDatabase : public IDatabase
	{
	private:
		std::string ConnectionString;
		std::vector<IDatabaseInstance*> Instances;
	public:

		virtual void setConnectionString(const std::string &cs);

		virtual IDatabaseInstance *open();
		virtual void closeAll();

		virtual std::string now();

		void removeInstance(IDatabaseInstance *i);
	};
}

#endif
