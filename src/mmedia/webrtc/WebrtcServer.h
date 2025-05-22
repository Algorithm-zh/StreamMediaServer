#pragma once
#include "network/UdpServer.h"
#include "network/net/UdpSocket.h"
#include "network/net/EventLoop.h"
#include "WebrtcHandler.h"

#include <cstdint>
#include <memory>

namespace tmms
{
  namespace mm
  {
    using namespace tmms::network;
    class WebrtcServer
    {
    public:
      WebrtcServer(EventLoop *loop, const InetAddress &server, WebrtcHandler *handler);
      ~WebrtcServer() = default;
      void Start();

    private:
      void MessageCallback(const UdpSocketPtr &socket, const InetAddress &addr, MsgBuffer &buf);
      bool IsDtls(MsgBuffer &buf);
      bool IsStun(MsgBuffer &buf);
      bool IsRtp(MsgBuffer &buf);
      bool IsRtcp(MsgBuffer &buf);

      WebrtcHandler *handler_{nullptr};
      std::shared_ptr<UdpServer> udp_server_;
    };
  }
}
