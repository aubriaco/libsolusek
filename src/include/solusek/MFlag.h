/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __M_FLAG_INCLUDED__
#define __M_FLAG_INCLUDED__
#include "EFlag.h"
#include <string>

namespace solusek
{

	class MFlag
	{
	public:

		MFlag() {}
		MFlag(EFlag flag, const std::string &param)
		{
			Flag = flag;
			Param = param;
		}

		EFlag Flag;
		std::string Param;
	};

}

#endif
