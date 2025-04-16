#pragma once
#include "mmedia/rtmp/RtmpHandler.h"
#include "network/net/TcpConnection.h"
#include "network/TcpServer.h"

namespace tmms
{
    namespace mm
    {
        using namespace tmms::network;
        class RtmpServer:public TcpServer
        {
        public:
            RtmpServer(EventLoop *loop,const InetAddress &local,RtmpHandler *handler=nullptr);
            ~RtmpServer();

            void Start() override;
            void Stop() override;

        private:
            void OnNewConnection(const TcpConnectionPtr &conn);
            void OnDestroyed(const TcpConnectionPtr &conn);
            void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf);
            void OnWriteComplete(const ConnectionPtr &con);
            void OnActive(const ConnectionPtr &conn);
            RtmpHandler *rtmp_handler_{nullptr};
        };
    }
}