#include "WebrtcServer.h"
#include "mmedia/base/MMediaLog.h"
#include <functional>
#include <memory>
using namespace tmms::mm;


 
WebrtcServer::WebrtcServer(EventLoop *loop, const InetAddress &server, WebrtcHandler *handler)
:handler_(handler)
{
  udp_server_ = std::make_shared<UdpServer>(loop, server); 
}
 
void WebrtcServer::Start()  {
  udp_server_->SetRecvMsgCallback(std::bind(&WebrtcServer::MessageCallback, this,
                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  udp_server_->Start();
  WEBRTC_DEBUG << "WebrtcServer start";
}
 
void WebrtcServer::MessageCallback(const UdpSocketPtr &socket, const InetAddress &addr, MsgBuffer &buf)  {
 
  if(handler_)
  {
    if(IsStun(buf))
    {
      handler_->OnStun(socket, addr, buf);
    }
    else if(IsDtls(buf))
    {
      handler_->OnDtls(socket, addr, buf);
    }
    else if(IsRtp(buf))
    {
      handler_->OnRtp(socket, addr, buf);
    }
    else if(IsRtcp(buf))
    {
      handler_->OnRtcp(socket, addr, buf);
    }
  }
}
 
bool WebrtcServer::IsDtls(MsgBuffer &buf)  {
  const char *data = buf.Peek();
  return buf.ReadableBytes() >= 13 && data[0] >= 20 && data[0] <= 63;
}
 
bool WebrtcServer::IsStun(MsgBuffer &buf)  {
  const char *data = buf.Peek();
  return buf.ReadableBytes() >= 20 && data[0] >= 0 && data[0] <= 3;
}
 
bool WebrtcServer::IsRtp(MsgBuffer &buf)  {
  const char *data = buf.Peek();
  return buf.ReadableBytes() >= 12 && data[0] &80 && !((unsigned char)data[1] >= 192 && (unsigned char)data[1] <= 223);
}
 
bool WebrtcServer::IsRtcp(MsgBuffer &buf)  {

  const char *data = buf.Peek();
  return buf.ReadableBytes() >= 12 && data[0] &80 && ((unsigned char)data[1] >= 192 && (unsigned char)data[1] <= 223);
}
