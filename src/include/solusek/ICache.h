#ifndef __I_CACHE_INCLUDED__
#define __I_CACHE_INCLUDED__
#include <string>

namespace solusek
{
    class ICache
    {
    public:
        virtual void dispose() = 0;

        virtual void set(const std::string& var, const std::string& val) = 0;

        virtual std::string get(const std::string& var) = 0;
        
    };
}

#endif