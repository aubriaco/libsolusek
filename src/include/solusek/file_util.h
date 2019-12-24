/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __FILE_UTIL_INCLUDED__
#define __FILE_UTIL_INCLUDED__
#include <string>
#include <vector>

namespace solusek
{
    std::string readTextFile(const std::string& fname);
    std::vector<unsigned char> readBinaryFile(const std::string& fname);
}

#endif
