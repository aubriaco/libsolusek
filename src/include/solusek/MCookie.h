/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef MCOOKIE_H
#define MCOOKIE_H

namespace solusek
{

    class MCookie
    {
    public:
        std::string Name, Value, Path;
        bool None;

        MCookie(bool none = false)
        {
          None = none;
        }

        MCookie(const std::string& name, const std::string& value)
        {
            Name = name;
            Value = value;
            None = false;
        }

    };

}

#endif // MCOOKIE_H
