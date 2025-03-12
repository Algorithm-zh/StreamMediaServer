#pragma once
#include "Connection.h"
#include "../base/InetAddress.h"
#include "../base/MsgBuffer.h"
#include <bits/types/struct_iovec.h>
#include <functional>
#include <memory>
#include <list>
#include <vector>
#include <sys/uio.h>

namespace tmms
{
  namespace network
  {
    class TcpConnection;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using CloseConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, MsgBuffer &)>;
    using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
    struct BufferNode
    {
      BufferNode(void *buf, size_t s):addr(buf),size(s){}
      void *addr{nullptr};
      size_t size{0};
    };
    using BufferNodePtr = std::shared_ptr<BufferNode>;
    struct TimeoutEntry;
    using TimeoutCallback = std::function<void(const TcpConnectionPtr&)>;
    class TcpConnection : public Connection
    {
    public:
      //构造函数
      TcpConnection(EventLoop *loop, int socketfd, const InetAddress &localAddr, const InetAddress &peerAddr);
      ~TcpConnection();
      //关闭事件成员函数
      void SetCloseCallback(const CloseConnectionCallback &cb);
      void SetCloseCallback(CloseConnectionCallback &&cb);
      void OnClose() override;
      void ForceClose() override;
      //读事件成员函数
      void OnRead() override;
      void SetRecvMsgCallback(const MessageCallback &cb);
      void SetRecvMsgCallback(MessageCallback &&cb);
      //出错事件成员函数
      void OnError(const std::string &msg) override;
      //写事件成员函数
      void OnWrite() override;
      void SetWriteCompleteCallback(const WriteCompleteCallback &cb);
      void SetWriteCompleteCallback(WriteCompleteCallback &&cb);
      void Send(std::list<BufferNodePtr> &list);//全部发送
      void Send(const char *buf, size_t size);//发送单独的缓冲区
      //超时事件成员函数
      void OnTimeout();
      void SetTimeoutCallback(int timeout, const TimeoutCallback &cb);//几秒钟之后执行cb
      void SetTimeoutCallback(int timeout, TimeoutCallback &&cb);//几秒钟之后执行cb
      void ExtendLife();//延长生命周期
      void EnableCheckIdleTimeout(int32_t max_time);//启动空闲事件检测
    private:
      void SendInLoop(std::list<BufferNodePtr> &list);
      void SendInLoop(const char *buf, size_t size);

      //关闭事件成员变量
      bool closed_{false};
      CloseConnectionCallback close_cb_;
      //读事件成员变量
      MsgBuffer message_buffer_;
      MessageCallback message_cb_;
      //写事件成员变量
      std::vector<struct iovec> io_vec_list_;
      WriteCompleteCallback write_complete_cb_;
      //超时事件成员变量
      std::weak_ptr<TimeoutEntry> timeout_entry_; //超时事件条目
      int32_t max_idle_time_{30}; //最大空闲事件
    };
    struct TimeoutEntry
    {
      TimeoutEntry(const TcpConnectionPtr &c):conn(c){}
      ~TimeoutEntry()
      {
        auto c = conn.lock();
        if(c)
        {
          c->OnTimeout();
        }
      }
      std::weak_ptr<TcpConnection> conn;
    };
  }
}
