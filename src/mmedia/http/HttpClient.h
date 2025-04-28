#pragma once
#include "mmedia/base/Packet.h"
#include "network/TcpClient.h"
#include "network/net/EventLoop.h"
#include "network/base/InetAddress.h"
#include "mmedia/http/HttpRequest.h"
#include "HttpHandler.h"
#include <functional>
#include <memory>

namespace tmms
{
  namespace mm
  {
    using TcpClientPtr = std::shared_ptr<TcpClient>;
    class HttpClient
    {
    public:
      HttpClient(EventLoop *loop, HttpHandler *handler);
      ~HttpClient(); 
      //回调函数
      void SetCloseCallback(const CloseConnectionCallback &cb);
      void SetCloseCallback(CloseConnectionCallback && cb);
      //其它函数
      void Get(const std::string &url);
      void Post(const std::string &url, const PacketPtr &packet);
    private:
      void OnWriteComplete(const TcpConnectionPtr & conn);
      void OnConnection(const TcpConnectionPtr & conn, bool connected);
      void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf);

      bool ParseUrl(const std::string &url);
      void CreateTcpClient();

      EventLoop *loop_{nullptr};
      InetAddress addr_;
      HttpHandler *handler_{nullptr};
      TcpClientPtr tcp_client_;
      std::string url_;
      bool is_post_{false};
      CloseConnectionCallback close_cb_;
      HttpRequestPtr request_;
      PacketPtr out_packet_;
    };
  }
}

