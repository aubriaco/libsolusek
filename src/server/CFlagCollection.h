/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_FLAG_COLLECTION_INCLUDED__
#define __C_FLAG_COLLECTION_INCLUDED__
#include "../include/solusek/IFlagCollection.h"
#include <vector>

namespace solusek
{
	class CFlagCollection : public IFlagCollection
	{
	private:
		std::vector<MFlag> List;
	public:
		CFlagCollection() {}

		virtual MFlag *get(EFlag flagId)
		{
			for(unsigned int n = 0; n < List.size(); n++)
			{
				if(List[n].Flag == flagId)
					return &List[n];
			}
			return 0;
		}

		virtual void add(MFlag flag)
		{
			List.push_back(flag);
		}

		virtual unsigned int size()
		{
			return List.size();
		}

		virtual void clear()
		{
			List.clear();
		}

		virtual void dispose()
		{
			delete this;
		}
	};
}

#endif
