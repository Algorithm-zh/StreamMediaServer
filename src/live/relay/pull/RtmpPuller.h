#pragma once
#include "base/AppInfo.h"
#include "mmedia/rtmp/RtmpClient.h"
#include "network/net/EventLoop.h"
#include "live/Session.h"
#include "live/relay/pull/Puller.h"
#include "base/Target.h"
#include "mmedia/rtmp/RtmpHandler.h"
namespace tmms
{
  namespace live
  {
    using namespace tmms::mm;
    using namespace tmms::network;
    using namespace tmms::base;
    class RtmpPuller : public RtmpHandler, public Puller
    {
    public:
      RtmpPuller(EventLoop *loop, Session *s, PullHandler *handler);
      ~RtmpPuller();
      //Puller成员函数
      bool Pull(const TargetPtr &target) override;
      //RtmpHandler成员函数
      void OnNewConnection(const TcpConnectionPtr &conn) override;
      void OnConnectionDestroy(const TcpConnectionPtr &conn) override;
      void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override;
      bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override;
      void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data) override{};
      void OnActive(const ConnectionPtr &conn) override{};

    private:
      TargetPtr target_;
      RtmpClient *rtmp_client_{nullptr};
     
    };
  }
}
