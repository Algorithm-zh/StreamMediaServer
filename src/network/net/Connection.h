#pragma once
#include "network/base/InetAddress.h"
#include "Event.h"
#include "EventLoop.h"
#include <functional>
#include <unordered_map>
#include <memory>
#include <atomic>

namespace tmms
{
    namespace network
    {
        enum 
        {
            kNormalContext = 0,
            kRtmpContext,
            kHttpContext,
            kUserContext,
            kFlvContext,
        };
        struct BufferNode
        {
            BufferNode(void *buf,size_t s)
            :addr(buf),size(s)
            {}
            void *addr{nullptr};
            size_t size{0};
        };
        using BufferNodePtr = std::shared_ptr<BufferNode>;        
        using ContextPtr = std::shared_ptr<void>;
        class Connection;
        using ConnectionPtr = std::shared_ptr<Connection>;
        using ActiveCallback = std::function<void(const ConnectionPtr&)>;
        class Connection:public Event
        {
        public:
            Connection(EventLoop *loop, 
                        int fd, 
                        const InetAddress &localAddr, 
                        const InetAddress &peerAddr);
            virtual ~Connection() = default;

            void SetLocalAddr(const InetAddress &local);
            void SetPeerAddr(const InetAddress &peer);
            const InetAddress &LocalAddr() const;
            const InetAddress &PeerAddr() const;

            void SetContext(int type,const std::shared_ptr<void> &context);
            void SetContext(int type,std::shared_ptr<void> &&context);
            template <typename T> std::shared_ptr<T> GetContext(int type) const
            {
                auto iter = contexts_.find(type);
                if(iter!=contexts_.end())
                {
                    return std::static_pointer_cast<T>(iter->second);
                }
                return std::shared_ptr<T>();
            }
            void ClearContext(int type);
            void ClearContext();
            void SetActiveCallback(const ActiveCallback &cb);
            void SetActiveCallback(ActiveCallback &&cb);
            void Active();
            void Deactive();
            virtual void ForceClose() = 0;
        private:
            std::unordered_map<int,ContextPtr> contexts_;
            ActiveCallback active_cb_;
            std::atomic<bool> active_{false};
        protected:
            InetAddress local_addr_;
            InetAddress peer_addr_;            
        };
    }
}