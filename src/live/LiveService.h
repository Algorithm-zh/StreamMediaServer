#pragma once
#include "mmedia/rtmp/RtmpHandler.h"
#include "mmedia/http/HttpHandler.h"
#include "network/net/Connection.h"
#include "network/TcpServer.h"
#include "network/net/EventLoopThreadPool.h"
#include "base/TaskMgr.h"
#include "base/Singleton.h"
#include "mmedia/webrtc/WebrtcServer.h"
#include "live/Session.h"
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace tmms
{
  namespace live
  {
    using namespace tmms::network;
    using namespace tmms::base;
    using SessionPtr = std::shared_ptr<Session>;
    class LiveService : public RtmpHandler, public HttpHandler 
    {
    public:
      LiveService() = default;
      ~LiveService() = default;

      SessionPtr CreateSession(const std::string &session_name);
      SessionPtr FindSession(const std::string &session_name);
      bool CloseSession(const std::string &session_name);
      void OnTimer(const TaskPtr &t);//检测session有没有超时 
      //网络回调成员函数
      void OnNewConnection(const TcpConnectionPtr &conn) override;
      void OnConnectionDestroy(const TcpConnectionPtr &conn) override;
      void OnActive(const ConnectionPtr &conn) override;
      //协议回调成员函数
      //rtmp回调成员函数
      bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override;
      bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override;
      void OnRecv(const TcpConnectionPtr &conn, PacketPtr &&data)override;
      void OnRecv(const TcpConnectionPtr &conn, const PacketPtr &data) override{};
      //http回调成员函数
      void OnSent(const TcpConnectionPtr &conn) override;
      bool OnSentNextChunk(const TcpConnectionPtr &conn) override;
      void OnRequest(const TcpConnectionPtr &conn, const HttpRequestPtr &req, const PacketPtr &packet) override;

      //其它成员函数
      void Start();
      void Stop();
      EventLoop *GetNextLoop();
    private:
      EventLoopThreadPool *pool_{nullptr};
      std::vector<TcpServer *> servers_;
      std::mutex lock_;
      std::unordered_map<std::string, SessionPtr> sessions_;

      std::shared_ptr<WebrtcServer> webrtc_server_{nullptr};//因为servers_存的都是tcp的，现在需要udp 的server
    };
    #define sLiveService tmms::live::Singleton<LiveService>::Instance()
  }
}


