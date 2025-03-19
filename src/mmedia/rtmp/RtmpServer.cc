#include "RtmpServer.h"
#include "RtmpHandShake.h"
#include "../base/MMediaLog.h"
using namespace tmms::mm;

 
RtmpServer::RtmpServer(EventLoop *loop, const InetAddress &local, RtmpHandler *handler)
:TcpServer(loop, local), rtmp_handler_(handler)  {
 
}
 
RtmpServer::~RtmpServer()  {
 
  Stop();
}
 
void RtmpServer::Start()  {
  
  TcpServer::SetActiveCallback(std::bind(&RtmpServer::OnActive, this, std::placeholders::_1));
  TcpServer::SetNewConnectionCallback(std::bind(&RtmpServer::OnNewConnection, this, std::placeholders::_1));
  TcpServer::SetDestroyConnectionCallback(std::bind(&RtmpServer::OnDestroyed, this, std::placeholders::_1));
  TcpServer::SetMessageCallback(std::bind(&RtmpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
  TcpServer::SetWriteCompleteCallback(std::bind(&RtmpServer::OnWriteComplete, this, std::placeholders::_1));
  TcpServer::Start();
}
 
void RtmpServer::Stop()  {
 
  TcpServer::Stop();
}
 
void RtmpServer::OnNewConnection(const TcpConnectionPtr &conn)  {
 
  if(rtmp_handler_)
  {
    rtmp_handler_->OnNewConnection(conn);
  }
  RtmpHandShakePtr shake = std::make_shared<RtmpHandShake>(conn);
  conn->SetContext(kRtmpContext, shake);
  shake->Start();
}
 
void RtmpServer::OnDestroyed(const TcpConnectionPtr &conn)  {
 
  if(rtmp_handler_)
  {
    rtmp_handler_->OnConnectionDestroy(conn);
  }
  conn->ClearContext(kRtmpContext);
}
 
void RtmpServer::OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf)  {
 
  RtmpHandShakePtr shake = conn->GetContext<RtmpHandShake>(kRtmpContext);
  if(shake)
  {
    int ret = shake->HandShake(buf);
    //握手成功
    if(ret == 0)
    {
      RTMP_TRACE << "host: " << conn->PeerAddr().ToIpPort() << ", handshake done\n";
    }
    else if(ret == -1)
    {
      conn->ForceClose();
    }
  }
}
 
void RtmpServer::OnWriteComplete(const ConnectionPtr &conn)  {
 
  RtmpHandShakePtr shake = conn->GetContext<RtmpHandShake>(kRtmpContext);
  if(shake)
  {
    shake->WriteComplete();
  }
}
 
void RtmpServer::OnActive(const ConnectionPtr &conn)  {

  if(rtmp_handler_)
  {
    rtmp_handler_->OnActive(conn);
  }
 
}
