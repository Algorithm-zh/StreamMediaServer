#pragma once
#include "User.h"
namespace tmms
{
  namespace live
  {
    class Stream;
    using StreamPtr = std::shared_ptr<Stream>;

    class PublishUser : public User
    {
    public:
      explicit PublishUser(const ConnectionPtr &ptr, const StreamPtr &stream)
      :User(ptr), stream_(stream)
      {

      }
      StreamPtr Stream();//需要把数据注入到流里面
    private:
      StreamPtr stream_;
    };
  }
}
