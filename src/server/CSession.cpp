#include "CSession.h"

namespace solusek
{
  static unsigned long int Incremental = 0;

  CSession::CSession()
  {
    if(Incremental >= 4294967295)
      Incremental = 0;
    char buf[128];
    sprintf(buf, "%lu%lu", time(0), Incremental);
    ID = buf;
    Incremental++;
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
      vv = new CVarVal();
    vv->Var = name;
    vv->Val = value;
    Items.push_back(vv);
  }

}
