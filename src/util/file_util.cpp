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
      std::vector<unsigned char> result;
      FILE* file = fopen(fname.c_str(), "rb");
      if (!file)
        return result;

      if (fseek(file, 0, SEEK_END) == 0)
      {
        const long fileSize = ftell(file);
        if (fileSize > 0 && fseek(file, 0, SEEK_SET) == 0)
        {
          result.resize(static_cast<size_t>(fileSize));
          const size_t bytesRead = fread(result.data(), 1, result.size(), file);
          result.resize(bytesRead);
        }
      }
      fclose(file);
      return result;
    }

}
