#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <bits/socket.h>
#include <string>
#include <fcntl.h>
#include <memory>
#include "../base/Network.h"
#include "../base/InetAddress.h"
#include <unistd.h>

namespace tmms
{
  namespace network
  {
    using InetAddressPtr = std::shared_ptr<InetAddress>;
    class SocketOpt
    {
    public:
      //构造函数
      SocketOpt(int sock, bool v6=false);
      ~SocketOpt()=default;
      static int CreateNonblockingTcpSocket(int family);
      static int CreateNonblockingUdpSocket(int family);
      //服务器流程
      int BindAddress(const InetAddress &localaddr);
      int Listen();
      int Accept(InetAddress *peeraddr);
      //客户端流程
      int Connect(const InetAddress &addr);
      //获取地址
      InetAddressPtr GetLocalAddr();
      InetAddressPtr GetPeerAddr();
      //使能函数
      void SetTcpNoDelay(bool on);
      void SetReuseAddr(bool on);
      void SetReusePort(bool on);
      void SetKeepAlive(bool on);
      void SetNonBlocking(bool on);

   private:
      int sock_{-1};
      bool is_v6_{false};
    };
  }   
}
