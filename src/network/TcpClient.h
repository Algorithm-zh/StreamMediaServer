#pragma once
#include "network/base/InetAddress.h"
#include "network/net/TcpConnection.h"
#include "network/net/EventLoop.h"
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
            kTcpConStatusDisConnected = 3,
        };
        using ConnectionCallback = std::function<void (const TcpConnectionPtr &con,bool)>;
        class TcpClient:public TcpConnection
        {
        public:
            TcpClient(EventLoop *loop,const InetAddress &server);
            virtual ~TcpClient();

            void SetConnectCallback(const ConnectionCallback &cb);
            void SetConnectCallback(ConnectionCallback &&cb);
            void Connect();


            void OnRead() override;
            void OnWrite() override;
            void OnClose() override;

            void Send(std::list<BufferNodePtr>&list);
            void Send(const char *buf,size_t size);
            
        private:
            void ConnectInLoop();
            void UpdateConnectionStatus();        
            bool CheckError();
            InetAddress server_addr_;
            int32_t status_{kTcpConStatusInit};
            ConnectionCallback connected_cb_;
        };
    }
}