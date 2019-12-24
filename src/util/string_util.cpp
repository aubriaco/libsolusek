#include "../include/solusek/string_util.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <ctime>
#include <string>

namespace solusek
{

    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    static std::string timetostr(time_t t)
    {
        std::time_t now = std::time(NULL);
        std::tm * ptm = std::localtime(&now);
        char buffer[32];
        std::strftime(buffer, 32, "%Y%m%d%H%M%SZ", ptm);
        return std::string(buffer);
    }

    std::string strtoupper(std::string str)
    {
        int leng=str.length();
        for(int i=0; i<leng; i++)
            if (97<=str[i]&&str[i]<=122)//a-z
                str[i]-=32;
        return str;
    }
    std::string strtolower(std::string str)
    {
        int leng=str.length();
        for(int i=0; i<leng; i++)
            if (65<=str[i]&&str[i]<=90)//A-Z
                str[i]+=32;
        return str;
    }

    std::string replaceAll(const std::string& source, const std::string& from, const std::string& to)
    {
        std::string newString;
        newString.reserve(source.length());  // avoids a few memory allocations

        std::string::size_type lastPos = 0;
        std::string::size_type findPos;

        while(std::string::npos != (findPos = source.find(from, lastPos)))
        {
            newString.append(source, lastPos, findPos - lastPos);
            newString += to;
            lastPos = findPos + from.length();
        }

        // Care for the rest after last occurrence
        newString += source.substr(lastPos);

        //source.swap(newString);
        return newString;
    }
}
