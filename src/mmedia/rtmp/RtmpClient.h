#pragma once

#include "network/TcpClient.h"
#include "network/net/EventLoop.h"
#include "network/base/InetAddress.h"
#include "mmedia/rtmp/RtmpHandler.h"
#include <functional>
#include <memory>

namespace tmms
{
    namespace mm
    {
        using namespace tmms::network;
        using TcpClientPtr = std::shared_ptr<TcpClient>;

        class RtmpClient
        {
        public:
            RtmpClient(EventLoop *loop,RtmpHandler *handler);
            ~RtmpClient();

            void SetCloseCallback(const CloseConnectionCallback &cb);
            void SetCloseCallback(CloseConnectionCallback &&cb);

            void Play(const std::string &url);
            void Publish(const std::string &url);
            void Send(PacketPtr &&data);
        private:  
            void OnWriteComplete(const TcpConnectionPtr &conn);
            void OnConnection(const TcpConnectionPtr& conn,bool connected);
            void OnMessage(const TcpConnectionPtr& conn,MsgBuffer &buf);        
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