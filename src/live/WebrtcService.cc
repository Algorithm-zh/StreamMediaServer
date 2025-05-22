#include "WebrtcService.h"
#include "live/base/LiveLog.h"
#include "mmedia/http/HttpRequest.h"
#include "mmedia/http/HttpTypes.h"
#include "mmedia/http/HttpUtils.h"
#include "mmedia/http/HttpContext.h"

using namespace tmms::live;
 
void WebrtcService::OnStun(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf)  {
 
}
 
void WebrtcService::OnDtls(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf)  {
 
}
 
void WebrtcService::OnRtp(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf)  {
 
}
 
void WebrtcService::OnRtcp(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf)  {
 
}
 
void WebrtcService::OnRequest(const TcpConnectionPtr &conn, const HttpRequestPtr &req, const PacketPtr &packet)  {
  auto http_cxt = conn->GetContext<HttpContext>(kHttpContext);
  if(!http_cxt)
  {
    LIVE_ERROR << "not found http context.something wrong";
    return ;
  }
  LIVE_DEBUG << "on request:" << req->AppendToBuffer();
  if(req->IsRequest())
  {
    LIVE_DEBUG << "req method:" << req->Method() << " path:" << req->Path();
  }
  else
  {
    LIVE_DEBUG << "res code:" << req->GetStatusCode() << " msg:" << HttpUtils::ParseStatusMessage(req->GetStatusCode());
  }
  auto headers = req->Headers();
  for(auto const &h : headers)
  {
    LIVE_DEBUG << "header:" << h.first << ":" << h.second;
  }

  if(req->IsRequest())
  {
    if(req->Method() == mm::kOptions)
    {
      auto ret = HttpRequest::NewHttpOptionsResponse();
      http_cxt->PostRequest(ret);
      return ;
    }
  }
}
