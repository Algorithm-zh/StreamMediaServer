#pragma once
#include "../../network/TcpClient.h"
#include "../../network/net/EventLoop.h"
#include "../../network/base/InetAddress.h"
#include "RtmpHandler.h"
#include <functional>
#include <memory>

namespace tmms
{
  namespace mm
  {
    using TcpClientPtr = std::shared_ptr<TcpClient>;
    class RtmpClient
    {
    public:
      RtmpClient(EventLoop *loop, RtmpHandler *handler);
      ~RtmpClient(); 
      //回调函数
      void SetCloseCallback(const CloseConnectionCallback &cb);
      void SetCloseCallback(CloseConnectionCallback && cb);
      //其它函数
      void Play(const std::string &url);//启动拉流
      void Publish(const std::string &url);//启动推流
    private:
      void OnWriteComplete(const TcpConnectionPtr & conn);
      void OnConnection(const TcpConnectionPtr & conn, bool connected);
      void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf);

      bool ParseUrl(const std::string &url);
      void CreateTcpClient();

      EventLoop *loop_{nullptr};
      InetAddress addr_;
      RtmpHandler *handler_{nullptr};
      TcpClientPtr tcp_client_;
      std::string url_;
      bool is_player_{false};
      CloseConnectionCallback close_cb_;
    };
  }
}

