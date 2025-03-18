#pragma once
#include <string>
#include <vector>
namespace tmms
{
  namespace base
  {
    class StringUtils
    {
    public:
      //判断字符串是否以某个字符串开头
      static bool StartsWith(const std::string &str, const std::string &prefix);
      //判断字符串是否以某个字符串结尾
      static bool EndsWith(const std::string &str, const std::string &suffix);
      //获取文件路径
      static std::string FilePath(const std::string &path);
      //获取文件名和扩展名
      static std::string FileNameExt(const std::string &path);
      //获取文件名
      static std::string FileName(const std::string &path);
      //获取文件扩展名
      static std::string Extension(const std::string &path);
      // 分割字符串
      static std::vector<std::string> SplitString(const std::string &str, const std::string &delim);
      // 使用有限状态机
      static std::vector<std::string> SplitStringFSM(const std::string &str, const char delim);
    };
  }
}
