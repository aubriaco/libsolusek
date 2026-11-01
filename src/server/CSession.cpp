#include "CSession.h"
#include <atomic>
#include <cstdio>

namespace solusek
{
  static std::atomic<unsigned long long> Incremental(0);

  CSession::CSession()
  {
    const unsigned long long sequence = Incremental.fetch_add(1, std::memory_order_relaxed);
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%lld-%llu",
                  static_cast<long long>(time(0)), sequence);
    ID = buf;
  }

  CSession::~CSession()
  {
    for(std::vector<CVarVal*>::iterator it = Items.begin(); it != Items.end(); ++it)
    {
      delete (*it);
    }
  }

  std::string CSession::getStringValue(const std::string &name)
  {
    CVarVal *vv = get(name);
    if(vv)
      return vv->Val;
    return std::string();
  }

  CVarVal *CSession::get(const std::string &name)
  {
    for(std::vector<CVarVal*>::iterator it = Items.begin(); it != Items.end(); ++it)
    {
      if((*it)->Var == name)
        return (*it);
    }
    return 0;
  }

  void CSession::setStringValue(const std::string& name, const std::string& value)
  {
    CVarVal *vv = get(name);
    if(!vv)
    {
      vv = new CVarVal();
      Items.push_back(vv);
    }
    vv->Var = name;
    vv->Val = value;
  }

}
