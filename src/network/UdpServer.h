#pragma once
#include "net/UdpSocket.h"
#include "net/EventLoop.h"
#include "base/InetAddress.h"
#include "base/SocketOpt.h"

namespace tmms
{
  namespace network
  {
    class UdpServer : public UdpSocket
    {
    public:
      UdpServer(EventLoop *loop, const InetAddress &server);
      virtual ~UdpServer();
      void Start();
      void Stop();
    private:
      void Open();

      InetAddress server_;
    };
  }
}

 
