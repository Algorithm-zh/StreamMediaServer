#pragma once

#include <string>
#include <vector>

namespace tmms
{
    namespace base
    {
        using std::string;
        class StringUtils
        {
        public:
            static bool StartsWith(const string &s, const string &sub);
            static bool EndsWith(const string &s, const string &sub);
            static std::string FilePath(const std::string &path);
            static std::string FileNameExt(const std::string &path);
            static std::string FileName(const std::string &path) ;
            static std::string Extension(const std::string &path);
            static std::vector<std::string> SplitString(const string &s,const string &delimiter);
            static std::vector<std::string> SplitStringWithFSM(const string &s,const char delimiter);
        };
    }
}