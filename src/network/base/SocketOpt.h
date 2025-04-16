#pragma once
#include "InetAddress.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory>

namespace tmms
{
    namespace network
    {
        using InetAddressPtr = std::shared_ptr<InetAddress>;
        class SocketOpt
        {
        public:
            SocketOpt(int sock,bool v6=false);
            ~SocketOpt() = default;

            static int CreateNonblockingTcpSocket(int family);
            static int CreateNonblockingUdpSocket(int family);

            int BindAddress(const InetAddress &localaddr);
            int Listen();
            int Accept(InetAddress *peeraddr);
            int Connect(const InetAddress &addr);

            InetAddressPtr GetLocalAddr();
            InetAddressPtr GetPeerAddr();

            void SetTcpNoDelay(bool on);
            void SetReuseAddr(bool on);
            void SetReusePort(bool on);
            void SetKeepAlive(bool on);
            void SetNonBlocking(bool on);
        private:
            int sock_{-1};
            bool is_v6_{false};
        };
    }
}