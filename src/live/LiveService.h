#pragma once

#include "network/net/Connection.h"
#include "network/TcpServer.h"
#include "network/net/EventLoopThreadPool.h"
#include "base/TaskMgr.h"
#include "base/Task.h"
#include "base/NonCopyable.h"
#include "base/Singleton.h"
#include "mmedia/rtmp/RtmpHandler.h"
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>

namespace tmms
{
    namespace live
    {
        using namespace tmms::network;
        using namespace tmms::base;
        using namespace tmms::mm;
        
        class Session;
        using SessionPtr = std::shared_ptr<Session>;

        class LiveService:public RtmpHandler
        {
        public:
            LiveService() = default;
            ~LiveService() = default;

            SessionPtr CreateSession(const std::string &session_name);
            SessionPtr FindSession(const std::string &session_name);
            bool CloseSession(const std::string &session_name); 
            void OnTimer(const TaskPtr &t);

            void OnNewConnection(const TcpConnectionPtr &conn) override;
            void OnConnectionDestroy(const TcpConnectionPtr &conn) override;
            void OnActive(const ConnectionPtr &conn) override;
            bool OnPlay(const TcpConnectionPtr &conn,const std::string &session_name, const std::string &param) override;
            bool OnPublish(const TcpConnectionPtr &conn,const std::string &session_name, const std::string &param) override;
            void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override;
            void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data) override{};
            
            void Start();
            void Stop();
            EventLoop *GetNextLoop();

        private:
            EventLoopThreadPool * pool_{nullptr};
            std::vector<TcpServer*> servers_;
            std::mutex lock_;
            std::unordered_map<std::string,SessionPtr> sessions_;
        };
        #define sLiveService tmms::base::Singleton<tmms::live::LiveService>::Instance()
    }
}