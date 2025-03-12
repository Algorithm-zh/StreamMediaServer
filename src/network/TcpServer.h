#pragma once
#include "base/InetAddress.h"
#include "net/EventLoop.h"
#include "net/TcpConnection.h"
#include "net/Acceptor.h"
#include <functional>
#include <memory>
#include <unordered_set>

namespace tmms
{
  namespace network
  {
    using NewConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    using DestroyConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
    class TcpServer
    {
    public:
      //构造函数
      TcpServer(EventLoop *loop, const InetAddress &addr);
      virtual ~TcpServer();
      //连接相关函数
      void SetNewConnectionCallback(const NewConnectionCallback &cb);
      void SetNewConnectionCallback(NewConnectionCallback &&cb);
      void SetDestroyConnectionCallback(const DestroyConnectionCallback &cb);
      void SetDestroyConnectionCallback(DestroyConnectionCallback &&cb);
      void OnAccet(int fd, const InetAddress &addr);
      void OnConnectionClose(const TcpConnectionPtr &con);
      //其它函数
      void SetActiveCallback(const ActiveCallback &cb);
      void SetActiveCallback(ActiveCallback &&cb);
      void SetWriteCompleteCallback(const WriteCompleteCallback &cb);
      void SetWriteCompleteCallback(WriteCompleteCallback &&cb);
      void SetMessageCallback(const MessageCallback &cb);
      void SetMessageCallback(MessageCallback &&cb);
      virtual void Start();
      virtual void Stop();

    private:
      EventLoop *loop_{nullptr};
      InetAddress addr_;
      std::shared_ptr<Acceptor> acceptor_;
      NewConnectionCallback new_connection_cb_;
      std::unordered_set<TcpConnectionPtr> connections_;
      MessageCallback message_cb_;
      ActiveCallback active_cb_;
      WriteCompleteCallback write_complete_cb_;
      DestroyConnectionCallback destroy_connection_cb_;
    };
  }
}
