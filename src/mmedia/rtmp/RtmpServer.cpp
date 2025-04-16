#include "RtmpServer.h"
#include "mmedia/base/MMediaLog.h"
#include "RtmpContext.h"

using namespace tmms::mm;

RtmpServer::RtmpServer(EventLoop *loop,const InetAddress &local,RtmpHandler *handler)
:TcpServer(loop,local),rtmp_handler_(handler)
{

}
RtmpServer::~RtmpServer()
{
    Stop();
}

void RtmpServer::Start()
{
    TcpServer::SetActiveCallback(std::bind(&RtmpServer::OnActive,this,std::placeholders::_1));
    TcpServer::SetDestroyConnectionCallback(std::bind(&RtmpServer::OnDestroyed,this,std::placeholders::_1));
    TcpServer::SetNewConnectionCallback(std::bind(&RtmpServer::OnNewConnection,this,std::placeholders::_1));
    TcpServer::SetWriteCompleteCallback(std::bind(&RtmpServer::OnWriteComplete,this,std::placeholders::_1));
    TcpServer::SetMessageCallback(std::bind(&RtmpServer::OnMessage,this,std::placeholders::_1,std::placeholders::_2));
    TcpServer::Start();
    RTMP_DEBUG << "RtmpServer Start";
}
void RtmpServer::Stop()
{
    TcpServer::Stop();
}

void RtmpServer::OnNewConnection(const TcpConnectionPtr &conn)
{
    if(rtmp_handler_)
    {
        rtmp_handler_->OnNewConnection(conn);
    }
    RtmpContextPtr shake = std::make_shared<RtmpContext>(conn,rtmp_handler_);
    conn->SetContext(kRtmpContext,shake);
    shake->StartHandShake();
}
void RtmpServer::OnDestroyed(const TcpConnectionPtr &conn)
{
    if(rtmp_handler_)
    {
        rtmp_handler_->OnConnectionDestroy(conn);
    }
    conn->ClearContext(kRtmpContext);
}
void RtmpServer::OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf)
{
    RtmpContextPtr shake = conn->GetContext<RtmpContext>(kRtmpContext);
    if(shake)
    {
        int ret = shake->Parse(buf);
        if(ret == 0)
        {
            RTMP_TRACE << "host: " << conn->PeerAddr().ToIpPort() << " handshake success.";
        }
        else if(ret == -1)
        {
            conn->ForceClose();
        }
    }
}
void RtmpServer::OnWriteComplete(const ConnectionPtr &conn)
{
    RtmpContextPtr shake = conn->GetContext<RtmpContext>(kRtmpContext);
    if(shake)
    {
        shake->OnWriteComplete();
    }
}
void RtmpServer::OnActive(const ConnectionPtr &conn)
{
    if(rtmp_handler_)
    {
        rtmp_handler_->OnActive(conn);
    }
}