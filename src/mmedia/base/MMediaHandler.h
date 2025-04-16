#pragma once
#include "base/NonCopyable.h"
#include "network/net/TcpConnection.h"
#include "Packet.h"
#include <memory>

namespace tmms
{
    namespace mm
    {
        using namespace network;
        class MMediaHandler:public base::NonCopyable
        {
        public:
            virtual void OnNewConnection(const TcpConnectionPtr &conn) = 0;
            virtual void OnConnectionDestroy(const TcpConnectionPtr &conn) = 0;
            virtual void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data) = 0;
            virtual void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) = 0;
            virtual void OnActive(const ConnectionPtr &conn) = 0;
        };        
    }
}