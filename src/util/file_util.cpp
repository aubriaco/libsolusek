#include "../include/solusek/file_util.h"

namespace solusek
{
    std::string readTextFile(const std::string& fname)
    {
      std::string r;
      FILE* file = fopen(fname.c_str(), "r");
      if(file)
      {
        fseek(file, 0, SEEK_END);
        size_t len = ftell(file);
        if(len > 0)
        {
          fseek(file, 0, SEEK_SET);
          char *buf = new char[len];
          fread(buf, 1, len, file);
          r = std::string(buf, buf + len);
          delete[] buf;
        }
        fclose(file);
      }
      return r;
    }

    std::vector<unsigned char> readBinaryFile(const std::string& fname)
    {
      std::vector<unsigned char> r;
      FILE* file = fopen(fname.c_str(), "r");
      if(file)
      {
        fseek(file, 0, SEEK_END);
        size_t len = ftell(file);
        fseek(file, 0, SEEK_SET);
        unsigned int rd = 512;
        char buf[512];
        for(unsigned int n = 0; n < len; n += 512)
        {
            if(n + 512 > len)
                rd = len - n;
            fread(buf, 1, rd, file);
            r.insert(r.end(), buf, buf + rd);
        }
      }
      return r;
    }

}
