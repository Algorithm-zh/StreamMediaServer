#pragma once
#include "net/TcpConnection.h"
#include "base/InetAddress.h"
#include "net/EventLoop.h"
#include <cstdint>
#include <functional>
namespace tmms
{
  namespace network
  {
    enum
    {
      kTcpConStatusInit = 0,
      kTcpConStatusConnecting = 1,
      kTcpConStatusConnected = 2,
      kTcpConStatusDisconnected = 3,
    };
    using ConnectionCallback = std::function<void(const TcpConnectionPtr &con, bool)> ;
    class TcpClient : public TcpConnection
    {
    public:
      TcpClient(EventLoop *loop, const InetAddress &server);
      virtual ~TcpClient();
      //连接成员函数
      void Connect();
      void SetConnectCallback(const ConnectionCallback &cb);
      void SetConnectCallback(ConnectionCallback &&cb);
      //IO事件成员函数
      void OnRead() override;
      void OnWrite() override;
      void OnClose() override;
      //写数据成员函数
      void Send(std::list <BufferNodePtr> &list);
      void Send(const char *buf, size_t size);
    private:
      void ConnectInLoop();
      void UpdateConnectionStatus();
      bool CheckError();//检查套接字有无错误
      InetAddress server_addr_;
      int32_t status_;
      ConnectionCallback connected_cb_;
    };
  }
}
