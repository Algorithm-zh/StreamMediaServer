#pragma once
#include "network/base/InetAddress.h"
#include "network/base/MsgBuffer.h"
#include "network/net/EventLoop.h"
#include "network/net/Connection.h"
#include <list>
#include <functional>
#include <memory>

namespace tmms
{
    namespace network
    {
        class UdpSocket;
        using UdpSocketPtr = std::shared_ptr<UdpSocket>;
        using UdpSocketMessageCallback = std::function<void(const InetAddress &addr,MsgBuffer &buf)>;
        using UdpSocketWriteCompleteCallback = std::function<void(const UdpSocketPtr &)>;
        using UdpSocketCloseConnectionCallback = std::function<void(const UdpSocketPtr &)>;
        using UdpSocketTimeoutCallback = std::function<void(const UdpSocketPtr &)>;
        struct UdpTimeoutEntry;
        struct UdpBufferNode:public BufferNode
        {
            UdpBufferNode(void *buf,size_t s,struct sockaddr* saddr,socklen_t len)
            :BufferNode(buf,s),sock_addr(saddr),sock_len(len)
            {}
            struct sockaddr *sock_addr{nullptr};
            socklen_t sock_len{0};
        };
        using UdpBufferNodePtr = std::shared_ptr<UdpBufferNode>;  
        class UdpSocket:public Connection
        {
        public:
            UdpSocket(EventLoop *loop, 
                int socketfd, 
                const InetAddress &localAddr, 
                const InetAddress &peerAddr);
            ~UdpSocket();
            void SetCloseCallback(const UdpSocketCloseConnectionCallback &cb);
            void SetCloseCallback(UdpSocketCloseConnectionCallback &&cb);
            void SetRecvMsgCallback(const UdpSocketMessageCallback &cb);
            void SetRecvMsgCallback(UdpSocketMessageCallback &&cb);
            void SetWriteCompleteCallback(const UdpSocketWriteCompleteCallback &cb);
            void SetWriteCompleteCallback(UdpSocketWriteCompleteCallback &&cb);
            void SetTimeoutCallback(int timeout, const UdpSocketTimeoutCallback &cb);
            void SetTimeoutCallback(int timeout, UdpSocketTimeoutCallback &&cb);
            void OnTimeOut();
            void OnError(const std::string &msg) override;
            void OnRead() override;
            void OnWrite() override;
            void OnClose() override;
            void EnableCheckIdleTimeout(int32_t max_time);
            void Send(std::list<UdpBufferNodePtr>&list);
            void Send(const char *buf,size_t size,struct sockaddr * addr,socklen_t len);
            void ForceClose() override;
        private:
            void ExtendLife();
            void SendInLoop(std::list<UdpBufferNodePtr>&list);
            void SendInLoop(const char *buf,size_t size,struct sockaddr*saddr,socklen_t len);
            std::list<UdpBufferNodePtr> buffer_list_; 
            bool closed_{false};    
            int32_t max_idle_time_{30};
            std::weak_ptr<UdpTimeoutEntry> timeout_entry_;  
            int32_t message_buffer_size_{65535}; 
            MsgBuffer message_buffer_; 
            UdpSocketMessageCallback message_cb_;  
            UdpSocketWriteCompleteCallback write_complete_cb_; 
            UdpSocketCloseConnectionCallback close_cb_;  
        };
        struct UdpTimeoutEntry
        {
            UdpTimeoutEntry(const UdpSocketPtr &c)
            :conn(c)
            {

            }
            ~UdpTimeoutEntry()
            {
                auto c = conn.lock();
                if(c)
                {
                    c->OnTimeOut();
                }
            }
            std::weak_ptr<UdpSocket> conn;
        };        
    }
}