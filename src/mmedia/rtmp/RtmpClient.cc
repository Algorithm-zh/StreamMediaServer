#include "RtmpClient.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/rtmp/RtmpContext.h"

using namespace tmms::mm;
using namespace tmms::network;

RtmpClient::RtmpClient(EventLoop *loop,RtmpHandler *handler)
:loop_(loop),handler_(handler)
{

}
RtmpClient::~RtmpClient()
{

}

void RtmpClient::OnWriteComplete(const TcpConnectionPtr &conn)
{
    auto context = conn->GetContext<RtmpContext>(kRtmpContext);
    if(context)
    {
        context->OnWriteComplete();
    }
}
void RtmpClient::OnConnection(const TcpConnectionPtr& conn,bool connected)
{
    if(connected)
    {
        auto context = std::make_shared<RtmpContext>(conn,handler_,true);
        if(is_player_)
        {
            context->Play(url_);
        }
        else 
        {
            context->Publish(url_);
        }
        conn->SetContext(kRtmpContext,context);
        context->StartHandShake();
    }
}
void RtmpClient::OnMessage(const TcpConnectionPtr& conn,MsgBuffer &buf)
{
    auto context = conn->GetContext<RtmpContext>(kRtmpContext);
    if(context)
    {
        auto ret = context->Parse(buf);
        if(ret == -1)
        {
            RTMP_ERROR << "message parse error.";
            conn->ForceClose();
        }
    }
}
void RtmpClient::SetCloseCallback(const CloseConnectionCallback &cb)
{
    close_cb_ = cb;
}
void RtmpClient::SetCloseCallback(CloseConnectionCallback &&cb)
{
    close_cb_ = std::move(cb);
}

void RtmpClient::Play(const std::string &url)
{
    is_player_ = true;
    url_ = url;
    CreateTcpClient();
}
void RtmpClient::Publish(const std::string &url)
{
    is_player_ = false;
    url_ = url;
    CreateTcpClient();
}
bool RtmpClient::ParseUrl(const std::string &url)
{
    if(url.size()>7)//rtmp://
    {
        
        uint16_t port = 1935;

        auto pos = url.find_first_of(":/",7);
        if(pos!=std::string::npos)
        {
            std::string domain = url.substr(7,pos-7);
            if(url.at(pos) == ':')
            {
                auto pos1 = url.find_first_of("/",pos+1);
                if(pos1 != std::string::npos)
                {
                    port = std::atoi(url.substr(pos+1,pos1-pos).c_str());
                }
            }
            addr_.SetAddr(domain);
            addr_.SetPort(port);
            return true;
        }
    }
    return false;
}
void RtmpClient::CreateTcpClient()
{
    auto ret = ParseUrl(url_);
    if(!ret)
    {
        RTMP_ERROR << "invalid url:" << url_;
        if(close_cb_)
        {
            close_cb_(nullptr);
        }
        return;
    }
    tcp_client_ = std::make_shared<TcpClient>(loop_,addr_);
    tcp_client_->SetWriteCompleteCallback(std::bind(&RtmpClient::OnWriteComplete,this,std::placeholders::_1));
    tcp_client_->SetRecvMsgCallback(std::bind(&RtmpClient::OnMessage,this,std::placeholders::_1,std::placeholders::_2));
    tcp_client_->SetCloseCallback(close_cb_);
    tcp_client_->SetConnectCallback(std::bind(&RtmpClient::OnConnection,this,std::placeholders::_1,std::placeholders::_2));
    tcp_client_->Connect();
}           
