#pragma once
namespace tmms
{
  namespace base
  {
    //基类
    class NonCopyable
    {
    protected:
      NonCopyable() = default;
      ~NonCopyable() = default;
      NonCopyable(const NonCopyable&) = delete;
      NonCopyable& operator=(const NonCopyable&) = delete;
    };
  }
}
