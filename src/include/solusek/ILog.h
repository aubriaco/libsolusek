/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __I_LOG_INCLUDED__
#define __I_LOG_INCLUDED__

namespace solusek
{
    class ILog
    {
        virtual void setEnabled(bool enabled) = 0;
    };
} // namespace solusek

#endif