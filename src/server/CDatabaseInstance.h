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

namespace solusek
{

	class CDatabaseInstance : public IDatabaseInstance
	{
	private:
		CDatabaseHandler H;
		bool Connected;
		void *D;
	public:
		CDatabaseInstance(void *d, const std::string &connectionString);
		~CDatabaseInstance();

		virtual bool isConnected() { return Connected; }

		virtual void dispose();

		virtual ITransaction *begin();
	};

}

#endif
