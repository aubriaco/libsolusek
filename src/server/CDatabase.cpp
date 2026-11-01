/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#include "CDatabase.h"
#include "CTransaction.h"
#include "CDatabaseInstance.h"
#include <unistd.h>
#include <pthread.h>
#include <stdexcept>

namespace solusek
{
	CDatabase::CDatabase()
	{
		ExpireSeconds = SOLUSEK_DBPOOL_EXPIRESECONDS;
		Running = true;
		TID = 0;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_create(&TID, &attr, poolThread, this);
		pthread_attr_destroy(&attr);
	}

	CDatabase::~CDatabase()
	{
		Running.store(false, std::memory_order_release);
		pthread_join(TID, 0);
		closeAll();
	}

	void *CDatabase::poolThread(void *param)
	{
		CDatabase *db = (CDatabase *)param;
		db->_poolThread();
		return 0;
	}

	void CDatabase::_poolThread()
	{
		while (Running.load(std::memory_order_acquire))
		{
			{
				std::lock_guard<std::mutex> guard(InstancesMutex);
				const time_t now = time(0);
				std::vector<IDatabaseInstance *>::iterator it = Instances.begin();
				while (it != Instances.end())
				{
					CDatabaseInstance *instance = static_cast<CDatabaseInstance *>(*it);
					if (!instance->inUse() && now - instance->getT() > ExpireSeconds)
					{
						fprintf(stdout, "DB Pool: Expired: %lu\n", instance->getID());
						delete instance;
						it = Instances.erase(it);
					}
					else
						++it;
				}
			}
			sleep(1);
		}
	}

	IDatabaseInstance *CDatabase::getUnusedInstance(const std::string &cs)
	{
		for (std::vector<IDatabaseInstance *>::iterator it = Instances.begin(); it != Instances.end(); ++it)
		{
			if (!((CDatabaseInstance *)(*it))->inUse() && cs == ((CDatabaseInstance *)(*it))->getConnectionString())
			{
				((CDatabaseInstance *)(*it))->putInUse();
				return (*it);
			}
		}
		return 0;
	}

	IDatabaseInstance *CDatabase::open()
	{
		IDatabaseInstance *di;
		std::lock_guard<std::mutex> guard(InstancesMutex);
		di = getUnusedInstance(ConnectionString);
		if (!di)
		{
			di = new CDatabaseInstance(this, ConnectionString);
			if (!di->isConnected())
			{
				delete di;
				throw std::runtime_error("Failed to open connection to database.");
			}
			Instances.push_back(di);
		}
		return di;
	}

	void CDatabase::closeAll()
	{
		std::lock_guard<std::mutex> guard(InstancesMutex);
		while (!Instances.empty())
		{
			delete (CDatabaseInstance *)Instances.back();
			Instances.pop_back();
		}
	}

	void CDatabase::setConnectionString(const std::string &cs)
	{
		std::lock_guard<std::mutex> guard(InstancesMutex);
		ConnectionString = cs;
	}

	std::string CDatabase::now()
	{
		time_t rawtime;
		struct tm *timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

		return std::string(buffer);
	}

	void CDatabase::removeInstance(IDatabaseInstance *i)
	{
		std::lock_guard<std::mutex> guard(InstancesMutex);
		for (std::vector<IDatabaseInstance *>::iterator it = Instances.begin(); it != Instances.end(); ++it)
		{
			if ((*it) == i)
			{
				Instances.erase(it);
				return;
			}
		}
	}
}
