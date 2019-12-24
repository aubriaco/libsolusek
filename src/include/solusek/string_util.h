/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef STRING_UTIL_H
#define STRING_UTIL_H
#include <string>
namespace solusek
{
    void trim(std::string &s);
    std::string strtoupper(std::string str);
    std::string strtolower(std::string str);
    std::string replaceAll(const std::string& source, const std::string& from, const std::string& to);
}
#endif // STRING_UTIL_H
