/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_DATABASE_HANDLE_INCLUDED__
#define __C_DATABASE_HANDLE_INCLUDED__
#include "../include/solusek/MDatabaseHandle.h"

namespace solusek
{
	class CDatabaseHandle : public MDatabaseHandle
	{
	private:
		CDatabaseResource *RS;
		std::string ID;
	public:
		CDatabaseHandle(CDatabaseResource *rs) : RS(rs) {}
		virtual ~CDatabaseHandle()
		{
			if(RS)
				delete RS;
		}

		virtual std::string getID()
		{
			if(RS)
				return RS->ID;
			else
				return ID;
		}

		virtual void setID(const std::string& id)
		{
			if(RS)
				RS->ID = id;
			ID = id;
		}

		virtual std::vector< std::vector<std::string> > getData()
		{
			return RS->Data;
		}

		virtual unsigned int size()
		{
			if(RS)
				return RS->Data.size();
			else
				return 0;
		}

		virtual void dispose()
		{
			delete this;
		}

		virtual std::string disposeAndGetError()
		{
			std::string error = RS->Error;
			dispose();
			return error;
		}

		virtual std::string disposeAndGetID()
		{
			std::string id = RS->ID;
			dispose();
			return id;
		}

		virtual bool isError()
		{
			return RS->isError();
		}

		virtual std::string getError()
		{
			return RS->Error;
		}

	};
}

#endif
