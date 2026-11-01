/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_VAR_VAL_INCLUDED__
#define __I_VAR_VAL_INCLUDED__
#include <string>

namespace solusek
{
    class IVarVal
    {
    public:
        virtual const std::string &getVar() = 0;
        virtual const std::string &getVal() = 0;
        virtual void dispose() = 0;
    };
}

#endif
