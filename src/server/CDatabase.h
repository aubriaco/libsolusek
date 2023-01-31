/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_DATABASE_INCLUDED__
#define __C_DATABASE_INCLUDED__
#include <pthread.h>
#include "../handlers/CDatabaseHandler.h"
#include "../include/solusek/IDatabase.h"
#include "../include/solusek/IDatabaseInstance.h"

#define SOLUSEK_DBPOOL_EXPIRESECONDS 30

namespace solusek
{
	class CDatabase : public IDatabase
	{
	private:
		std::string ConnectionString;
		std::vector<IDatabaseInstance*> Instances;
		bool Running, Hold;
		pthread_t TID;
		int ExpireSeconds;
	public:
		CDatabase();
		~CDatabase();

		static void *poolThread(void *param);
		void _poolThread();

		IDatabaseInstance *getUnusedInstance(const std::string& cs);

		virtual void setConnectionString(const std::string &cs);

		virtual IDatabaseInstance *open();
		virtual void closeAll();

		virtual std::string now();

		void removeInstance(IDatabaseInstance *i);
	};
}

#endif
