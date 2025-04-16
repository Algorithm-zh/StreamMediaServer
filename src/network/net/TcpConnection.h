#pragma once

#include "Connection.h"
#include "network/base/InetAddress.h"
#include "network/base/MsgBuffer.h"
#include <functional>
#include <memory>
#include <list>
#include <sys/uio.h>

namespace tmms
{
    namespace network
    {
        class TcpConnection;
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using CloseConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
        using MessageCallback = std::function<void(const TcpConnectionPtr &,MsgBuffer &buffer)>;
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
        using TimeoutCallback = std::function<void(const TcpConnectionPtr &)>;

        struct TimeoutEntry;
        class TcpConnection:public Connection
        {
        public:
            TcpConnection(EventLoop *loop, 
                        int socketfd, 
                        const InetAddress &localAddr, 
                        const InetAddress &peerAddr);
            virtual ~TcpConnection();

            void SetCloseCallback(const CloseConnectionCallback &cb);
            void SetCloseCallback(CloseConnectionCallback &&cb);
            void SetRecvMsgCallback(const MessageCallback &cb);
            void SetRecvMsgCallback(MessageCallback &&cb);
            void SetWriteCompleteCallback(const WriteCompleteCallback &cb);
            void SetWriteCompleteCallback(WriteCompleteCallback &&cb);
            void SetTimeoutCallback(int timeout,const TimeoutCallback &cb);
            void SetTimeoutCallback(int timeout,TimeoutCallback &&cb);
            void OnClose() override;
            void ForceClose() override;
            void OnRead() override;
            void OnError(const std::string &msg) override;
            void OnWrite() override;
            void Send(std::list<BufferNodePtr>&list);
            void Send(const char *buf,size_t size);
            void OnTimeout();
            void EnableCheckIdleTimeout(int32_t max_time);
        private:
            void SendInLoop(const char *buf,size_t size);
            void SendInLoop(std::list<BufferNodePtr>&list);
            void ExtendLife();

            bool closed_{false};   
            CloseConnectionCallback close_cb_;   
            MsgBuffer message_buffer_;
            MessageCallback message_cb_;  
            std::vector<struct iovec> io_vec_list_;
            WriteCompleteCallback write_complete_cb_;
            std::weak_ptr<TimeoutEntry> timeout_entry_;
            int32_t max_idle_time_{30};
        };
        struct TimeoutEntry
        {
            TimeoutEntry(const TcpConnectionPtr &c)
            :conn(c)
            {

            }
            ~TimeoutEntry()
            {
                auto c = conn.lock();
                if(c)
                {
                    c->OnTimeout();
                }
            }
            std::weak_ptr<TcpConnection> conn;
        };

    }
}