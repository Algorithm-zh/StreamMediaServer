#pragma once
#include "base/NonCopyable.h"
#include "network/net/UdpSocket.h"
#include <cstdint>
namespace tmms
{
  namespace mm
  {
    class WebrtcHandler:public base::NonCopyable
    {
    public:
      WebrtcHandler() = default;
      ~WebrtcHandler() = default;
      virtual void OnStun(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf) = 0;
      virtual void OnDtls(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf) = 0;
      virtual void OnRtp(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf) = 0;
      virtual void OnRtcp(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf) = 0;
    };
  }
}
