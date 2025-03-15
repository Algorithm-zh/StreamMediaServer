#pragma once
#include "base/InetAddress.h"
#include "net/UdpSocket.h"
#include <functional>

namespace tmms
{
  namespace network
  {
    using ConnectedCallback = std::function<void(const UdpSocketPtr &sock, bool)> ;
    class UdpClient : public UdpSocket
    {
    public:
      UdpClient(EventLoop *loop, const InetAddress &server);
      virtual ~UdpClient();
      void Connect();
      void SetConnectedCallback(const ConnectedCallback &cb);
      void SetConnectedCallback(ConnectedCallback &&cb);
      void ConnectInLoop();
      void Send(std::list <BufferNodePtr> &list);
      void Send(const char *buf, size_t size);
      void OnClose() override;
    private:
      bool connected_{false};
      InetAddress server_addr_;
      ConnectedCallback connected_cb_;
      struct sockaddr_in6 sock_addr_;
      socklen_t sock_len_{sizeof(sock_addr_)};
    };
  }
}
