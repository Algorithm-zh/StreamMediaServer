#pragma once
#include "../base/InetAddress.h"
#include "Event.h"
#include <functional>
#include <unordered_map>
#include <memory>

namespace tmms
{
  namespace network
  {
    enum
    {
      kNormalContext = 0,
      kRtmpContext = 1,
      kHttpContext = 2,
      kUserContext = 3,
      kFlvContext = 4,
    };
    using ContextPtr = std::shared_ptr<void>;
    class Connection : public Event
    {
    public:
      Connection(EventLoop *loop, int fd, const InetAddress &localAddr, const InetAddress &peerAddr);
      ~Connection() = default;
    private:
      std::unordered_map<int, ContextPtr> contexts_;
    };
  }
}
