/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_VAR_VAL_INCLUDED__
#define __C_VAR_VAL_INCLUDED__
#include <string>
#include "../include/solusek/IVarVal.h"

namespace solusek
{
	class CVarVal : public IVarVal
	{
	public:
		std::string Var, Val;

		CVarVal() {}
		CVarVal(std::string var, std::string val) : Var(var), Val(val) {}
		virtual ~CVarVal() {}

		virtual const std::string &getVar() { return Var; }
		virtual const std::string &getVal() { return Val; }

		virtual void dispose() { delete this; }
	};
}

#endif
