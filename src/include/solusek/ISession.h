#ifndef __I_SESSION_INCLUDED__
#define __I_SESSION_INCLUDED__
#include <string>

namespace solusek
{
  class ISession
  {
  public:
    virtual std::string getID() = 0;
    virtual std::string getStringValue(const std::string &name) = 0;
    virtual void setStringValue(const std::string& name, const std::string& value) = 0;
  };
}

#endif
