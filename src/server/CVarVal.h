/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_VAR_VAL_INCLUDED__
#define __C_VAR_VAL_INCLUDED__
#include <string>

namespace solusek
{
	class CVarVal
	{
	public:
		std::string Var, Val;

		CVarVal() {}
		CVarVal(std::string var, std::string val) : Var(var), Val(val) {}
	};
}

#endif
