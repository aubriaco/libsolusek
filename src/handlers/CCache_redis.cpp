#ifdef USE_REDIS
#pragma message "Using REDIS"
#include "../server/CCache.h"

#include <hiredis/hiredis.h>

namespace solusek
{
    std::string CCache::Host = "localhost";
    int CCache::Port = 6379;

    CCache::CCache()
    {
        Context = redisConnect(Host.c_str(), Port);
    }

    CCache::~CCache()
    {
        redisFree((redisContext*)Context);
    }

    void CCache::dispose()
    {
        delete this;
    }

    void CCache::set(const std::string& var, const std::string& val)
    {
        redisReply* reply = (redisReply*)redisCommand((redisContext*)Context, "SET %s %s", var.c_str(), val.c_str());
        if(reply)
            freeReplyObject(reply);
    }

    std::string CCache::get(const std::string& var)
    {
        redisReply* reply = (redisReply*)redisCommand((redisContext*)Context, "GET %s", var.c_str());
        if(reply)
        {
            std::string ret(reply->str);
            freeReplyObject(reply);
            return ret;
        }
        else
            return std::string();
    }
}

#endif