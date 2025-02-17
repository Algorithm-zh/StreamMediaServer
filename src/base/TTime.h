#pragma once
#include <cstdint>
#include <string>
#include <sys/time.h>
namespace tmms{
  namespace base
  {
    class TTime
    {
    public:
      //当前的UTC时间，单位是毫秒
      static int64_t NowMs();
      static int64_t Now();
      static int64_t Now(int &year, int &month, int &day, int &hour, int &minute, int &second);
      //主要用于日志输出
      static std::string ISOTime();
    };
  }
}
