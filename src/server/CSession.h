#ifndef __C_SESSION_INCLUDED__
#define __C_SESSION_INCLUDED__
#include "../include/solusek/ISession.h"
#include "CVarVal.h"
#include <vector>

namespace solusek
{
  class CSession : public ISession
  {
  private:
    std::string ID;
    std::vector<CVarVal*> Items;
  public:
    CSession();
    ~CSession();
    virtual std::string getID() { return ID; }
    virtual std::string getStringValue(const std::string &name);
    virtual CVarVal *get(const std::string &name);
    virtual void setStringValue(const std::string& name, const std::string& value);
  };
}

#endif
