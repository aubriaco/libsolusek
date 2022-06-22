#ifndef __C_CACHE_INCLUDED__
#define __C_CACHE_INCLUDED__
#include "../include/solusek/ICache.h"

namespace solusek
{
    class CCache : public ICache
    {
    private:
        void* Context;
    public:
        static std::string Host;
        static int Port;

        CCache();
        ~CCache();

        virtual void dispose();

        virtual void set(const std::string& var, const std::string& val);

        virtual std::string get(const std::string& var);
    };
}

#endif