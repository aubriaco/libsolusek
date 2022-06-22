#ifndef USE_REDIS

#include "../server/CCache.h"


namespace solusek
{
    std::string CCache::Host = "localhost";
    int CCache::Port = 6379;


    CCache::CCache()
    {

    }

    CCache::~CCache()
    {
                
    }

    void CCache::dispose()
    {
        delete this;
    }

    void CCache::set(const std::string& var, const std::string& val)
    {

    }

    std::string CCache::get(const std::string& var)
    {
        return std::string();
    }
}

#endif