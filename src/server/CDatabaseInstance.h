/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_DATABASEINSTANCE_INCLUDED__
#define __C_DATABASEINSTANCE_INCLUDED__
#include "../include/solusek/IDatabaseInstance.h"
#include "../handlers/CDatabaseHandler.h"
#include <atomic>

namespace solusek
{

	class CDatabaseInstance : public IDatabaseInstance
	{
	private:
		static size_t Counter;
		size_t ID;
		CDatabaseHandler H;
		bool Connected;
		void *D;
		std::atomic<bool> Used;
		std::string ConnectionString;
		time_t T;

	public:
		CDatabaseInstance(void *d, const std::string &connectionString);
		~CDatabaseInstance();

		size_t getID() { return ID; }
		bool inUse() { return Used.load(std::memory_order_acquire); }
		void putInUse()
		{
			T = time(0);
			Used.store(true, std::memory_order_release);
		}
		std::string getConnectionString() { return ConnectionString; }
		time_t getT() { return T; }

		virtual bool isConnected() { return Connected; }

		virtual void dispose();

		virtual ITransaction *begin();
	};

}

#endif
