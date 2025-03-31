#include "StringUtils.h"
using namespace tmms::base;
bool StringUtils::StartsWith(const std::string &s, const std::string &sub)  {
  if(sub.empty())return true;
  if(s.empty())return false;
  auto len = s.size();
  auto slen = sub.size();
  if(slen > len)return false;
	return s.compare(0, slen, sub) == 0;
}
 
bool StringUtils::EndsWith(const std::string &s, const std::string &sub)  {
  if(sub.empty())return true;
  if(s.empty())return false;
  auto len = s.size();
  auto slen = sub.size();
  if(slen > len)return false;
	return s.compare(len - slen, slen, sub) == 0;
}
 
std::string StringUtils::FilePath(const std::string &path)  {
  //从后往前找，如果找到path中的任何一个字符，则返回其在str中的索引值.否则返回npos
  auto pos = path.find_last_of("/\\");
  if(pos != std::string::npos)
  {
    return path.substr(0, pos);
  }else{
    return "./";
  } 
}
 
std::string StringUtils::FileNameExt(const std::string &path)  {
  auto pos = path.find_last_of("/\\");
  if(pos != std::string::npos)
  {
    if(pos + 1 < path.size())
    {
      return path.substr(pos + 1);
    }
  }
  return path;
}
 
std::string StringUtils::FileName(const std::string &path)  {
  std::string file_name = FileNameExt(path);
  auto pos = file_name.find_last_of(".");
  if(pos != std::string::npos)
  {
    //如果第一个是.的话，不认为是文件名
    if(pos != 0)
      return file_name.substr(0, pos);
  }
  return file_name;
}
 
std::string StringUtils::Extension(const std::string &path)  {
  std::string file_name = FileNameExt(path);
  auto pos = file_name.find_last_of(".");
  if(pos != std::string::npos)
  {
    //查找不到点或者点后面没有字符
    if(pos != 0 && pos + 1 < file_name.size())
      return file_name.substr(pos + 1);
  }
  return file_name;

	return std::string();
}
 
std::vector<std::string> StringUtils::SplitString(const std::string &s, const std::string &delim)  {
  if(delim.empty())
  {
    return std::vector<std::string>();
  }
  std::vector<std::string> result;
  size_t last = 0;
  size_t next = 0;
  while((next = s.find(delim, last)) != std::string::npos)
  {
    if(next > last)
    {
      result.emplace_back(s.substr(last, next - last));
    }
    else
    {
      result.emplace_back("");
    }
    last = next + delim.size();
  }
  if(last < s.size())
  {
    result.emplace_back(s.substr(last));
  }
  return result;
}
 
std::vector<std::string> StringUtils::SplitStringFSM(const std::string &str, const char delim)  {

  enum 
  {
    kStateInit = 0,
    kStateNormal = 1,
    kStateDelimiter = 2,
    kStateEnd = 3,
  };
  std::vector<std::string> result;
  int state = kStateInit;
  std::string tmp;
  state = kStateNormal;
  for(int pos = 0; pos < str.size();)
  {
    if(state == kStateNormal)
    {
      if(str.at(pos) == delim)
      {
        state = kStateDelimiter;
        continue;
      }
      tmp.push_back(str.at(pos));
      pos++;
    }else if(state == kStateDelimiter)
    {
      result.push_back(tmp);
      tmp.clear();
      state = kStateNormal;
      pos ++;
    }
  }
  if(!tmp.empty())
  {
    result.push_back(tmp);
  }

  return result;
}




