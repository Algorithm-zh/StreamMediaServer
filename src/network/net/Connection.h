#pragma once
#include "../base/InetAddress.h"
#include "Event.h"
#include <functional>
#include <unordered_map>
#include <memory>
#include <atomic>
#include "EventLoop.h"

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
    class Connection;
    using ConnectionPtr = std::shared_ptr<Connection>;
    using ActiveCallback = std::function<void(const ConnectionPtr&)>;
    class Connection : public Event
    {
    public:
      Connection(EventLoop *loop, int fd, const InetAddress &localAddr, const InetAddress &peerAddr);
      ~Connection() = default;
      //地址相关函数
      void SetLocalAddr(const InetAddress &local);
      void SetPeerAddr(const InetAddress &peer);
      const InetAddress &LocalAddr() const;
      const InetAddress &PeerAddr() const;
      //私有数据相关函数
      void SetContext(int type, const std::shared_ptr<void> &context);
      void SetContext(int type, std::shared_ptr<void> &&context);
      template <typename T> std::shared_ptr<T> GetContext(int type) const
      {
        //contexts_认为是在一个事件循环里，所以不需要枷锁
        auto iter = contexts_.find(type);
        if(iter == contexts_.end())
        {
          return nullptr;
        }
        return std::dynamic_pointer_cast<T>(iter->second);
      }
      void ClearContext(int type);
      void ClearContext();
      //激活相关函数
      void SetActiveCallback(const ActiveCallback &cb);
      void SetActiveCallback(ActiveCallback &&cb);
      void Active();
      void Deactive();
      //关闭函数 由子类实现
      virtual void ForceClose() = 0;
    private:
      std::unordered_map<int, ContextPtr> contexts_;
      ActiveCallback active_cb_;
      std::atomic_bool active_{false};
    protected:
      InetAddress local_addr_;
      InetAddress peer_addr_;
    };
  }
}
