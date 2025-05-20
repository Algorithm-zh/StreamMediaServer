#include "RtmpPuller.h"
#include "live/base/LiveLog.h"
#include "mmedia/rtmp/RtmpClient.h"
#include "live/Stream.h"
#include "network/net/Connection.h"
#include "network/net/TcpConnection.h"
#include <memory>
#include <sstream>
#include <utility>
using namespace tmms::live;
RtmpPuller::RtmpPuller(EventLoop *loop, Session *s, PullHandler *handler)
:Puller(loop, s, handler)  
{
 
}
 
RtmpPuller::~RtmpPuller()  {
 
  if(rtmp_client_)
  {
    delete rtmp_client_;
    rtmp_client_ = nullptr;
  }
}
 
bool RtmpPuller::Pull(const TargetPtr &target)  {
  if(target->protocol == "RTMP" || target->protocol == "rtmp")
  {
    if(rtmp_client_)
    {
      delete rtmp_client_;
      rtmp_client_ = nullptr;
    }
    if(!rtmp_client_)
    {
      rtmp_client_ = new RtmpClient(event_loop_, this);
      rtmp_client_->SetCloseCallback([this](const TcpConnectionPtr &conn){
        if(pull_handler_)
        {
          pull_handler_->OnPullClose();
        }
      });
    }
    //组装session
    std::stringstream ss;
    ss << "rtmp://" << target->remote_host 
      << ":" << target->remote_port 
      << "/" << target->domain_name
      << "/" << target->app_name
      << "/" << target->stream_name;
    PULLER_DEBUG << "rtmp client pull:" << ss.str();
    target_ = target;
    //启动拉流
    rtmp_client_->Play(ss.str());
    return true;
  }
  return false;
}
 
void RtmpPuller::OnNewConnection(const TcpConnectionPtr &conn)  {
 
}
 
void RtmpPuller::OnConnectionDestroy(const TcpConnectionPtr &conn)  {
  UserPtr user = conn->GetContext<User>(kUserContext); 
  if(user)
  {
    if(session_)
    {
      session_->CloseUser(user);
    }
  }
}
 
void RtmpPuller::OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data)  {
 
  UserPtr user = conn->GetContext<User>(kUserContext);
  if(!user)
  {
    PULLER_ERROR << "no user found.host:" << conn->PeerAddr().ToIpPort();
    return;
  }
  //注入数据，将数据发送到流中
  session_->GetStream()->AddPacket(std::move(data));
}
 
bool RtmpPuller::OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param)  {
  UserPtr user = conn->GetContext<User>(kUserContext);
  if(!user)
  {
    user = session_->CreatePublishUser(std::dynamic_pointer_cast<Connection>(conn), session_name, "", UserType::kUserTypePlayerRtmp);
    if(!user)
    {
      PULLER_ERROR << "create user failed.";
      if(pull_handler_)
      {
        pull_handler_->OnPullClose();
      }
      return false;
    }
    conn->SetContext(kUserContext, user);
  }
  if(pull_handler_)
  {
    pull_handler_->OnPullSuccess();
  }
  session_->SetPublisher(user);
  return true;
}
