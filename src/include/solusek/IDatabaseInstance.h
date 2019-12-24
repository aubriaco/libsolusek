/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_DATABASEINSTANCE_INCLUDED__
#define __I_DATABASEINSTANCE_INCLUDED__
#include "ITransaction.h"

namespace solusek
{

	class IDatabaseInstance
	{
	public:
		virtual bool isConnected() = 0;
		virtual void dispose() = 0;
		virtual ITransaction *begin() = 0;
	};

}

#endif
