#ifndef __M_MIME_TYPE_INCLUDED__
#define __M_MIME_TYPE_INCLUDED__
#include <string>

namespace solusek
{
    class MMimeType
    {
    public:
      std::string Ext, Type;
      bool IsText;

      MMimeType(const std::string& ext, bool isText = true, const std::string& type = std::string())
      {
        Ext = ext;
        IsText = isText;
        if(type.size() == 0)
        {
          if(isText)
            Type = "text/" + ext;
          else
            Type = "application/" + ext;
        }
        else
          Type = type;
      }
    };
}

#endif
