#include "WebrtcService.h"
#include "base/StringUtils.h"
#include "live/LiveService.h"
#include "live/base/LiveLog.h"
#include "live/user/User.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/Packet.h"
#include "mmedia/http/HttpRequest.h"
#include "mmedia/http/HttpTypes.h"
#include "mmedia/http/HttpUtils.h"
#include "mmedia/http/HttpContext.h"
#include "live/Session.h"
#include "live/user/WebrtcPlayerUser.h"
#include "mmedia/webrtc/Stun.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/value.h"
#include <cstring>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace tmms::live;
 
void WebrtcService::OnStun(const network::UdpSocketPtr &socket, const network::InetAddress &addr, network::MsgBuffer &buf)  {
  LIVE_DEBUG << "stun msg:" << buf.ReadableBytes();
  //解析
  Stun stun;
  if(!stun.Decode(buf.Peek(), buf.ReadableBytes()))
  {
    LIVE_DEBUG << "stun decode error";
    return;
  }
  {
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = name_users_.find(stun.LocalUFrag());
    if(iter != name_users_.end())
    {
      auto webrtc_user = iter->second;
      name_users_.erase(iter);
      //更新密码
      stun.SetPassword(webrtc_user->LocalPasswd());
      
      users_.emplace(addr.ToIpPort(), webrtc_user);
    }
    else
    {
      return;
    }
  }
  stun.SetMessageType(mm::kStunMsgBindingResponse);
  stun.SetMappedAddr(addr.IPv4());
  stun.SetMappedPort(addr.Port());
  PacketPtr packet = stun.Encode();
  if(packet)
  {
    struct sockaddr_in6 sock_addr;
    addr.GetSockAddr((sockaddr*)&sock_addr);
    socket->Send(packet->Data(), packet->PacketSize(), (sockaddr*)&sock_addr, sizeof(struct sockaddr_in6));
  }
   
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
    if(req->Path() == "/rtc/v1/play/" && packet)
    {
      LIVE_DEBUG << "request:\n" << packet->Data();
      Json::CharReaderBuilder builder;
      Json::Value root;
      Json::String err;
      std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
      if(!reader->parse(packet->Data(), packet->Data() + packet->PacketSize(), &root, &err))
      {
        LIVE_ERROR << "parse json failed.error:" << err;
        auto res = HttpRequest::NewHttp404Response();
        http_cxt->PostRequest(res);
        return;
      }
      //获取参数
      auto api = root["api"].asString();
      auto streamurl = root["streamurl"].asString();
      auto clientip = root["clientip"].asString();//这是空的
      auto sdp = root["sdp"].asString();

      std::string session_name = GetSessionNameFromUrl(streamurl);
      LIVE_DEBUG << "get session name:" << session_name;

      auto s = sLiveService->CreateSession(session_name);
      if(!s)
      {
        LIVE_ERROR << "create session failed.session_name:" << session_name;
        auto res = HttpRequest::NewHttp404Response();
        http_cxt->PostRequest(res);
        return;
      }
      auto user = s->CreatePlayerUser(conn, session_name, "", UserType::kUserTypePlayerWebRTC);
      if(!user)
      {
        LIVE_ERROR << "create player user failed.session_name:" << session_name;
        auto res = HttpRequest::NewHttp404Response();
        http_cxt->PostRequest(res);
        return;
      }
      s->AddPlayer(std::dynamic_pointer_cast<PlayerUser>(user));
      auto webrtc_user = std::dynamic_pointer_cast<WebrtcPlayerUser>(user);
      if(webrtc_user == nullptr)
      {
        LIVE_ERROR << "create webrtc player user failed.session_name:" << session_name;
        auto res = HttpRequest::NewHttp404Response();
        http_cxt->PostRequest(res);
        return;
      }
      if(!webrtc_user->ProcessOfferSdp(sdp))
      {
        LIVE_ERROR << "process sdp failed.session_name:" << session_name;
        auto res = HttpRequest::NewHttp404Response();
        http_cxt->PostRequest(res);
        return;
      }
      //回复sdp
      auto answer_sdp = webrtc_user->BuildAnswerSdp();
      LIVE_DEBUG << "answer sdp:" << answer_sdp;
      Json::Value result;
      result["code"] = 0;
      result["server"] = "tmms";
      result["sdp"] = std::move(answer_sdp);
      result["sessionid"] = webrtc_user->RemoteUFrag() + ":" + webrtc_user->LocalUFrag();

      auto content = result.toStyledString();
      auto http_response = std::make_shared<HttpRequest>(false);
      auto res = std::make_shared<HttpRequest>(false);
      res->SetStatusCode(200);
      res->AddHeader("server", "tmms");
      res->AddHeader("content-length", std::to_string(content.size()));
      res->AddHeader("content-type", "text/plain");
      res->AddHeader("Access-Control-Allow-Origin", "*");
      res->AddHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
      res->AddHeader("Allow", "POST,GET,OPTIONS");
      res->AddHeader("Access-Control-Allow-Headers", "content-type");
      res->AddHeader("Connection", "close");
      res->SetBody(content);
      http_cxt->PostRequest(res);
      std::lock_guard<std::mutex> lk(lock_);
      name_users_.emplace(webrtc_user->LocalUFrag(), webrtc_user);
    }
  }
}
 
std::string WebrtcService::GetSessionNameFromUrl(const std::string &url)  {
  //webrtc://hx.com:8081/live/test
  //webrtc://hx.com:8081/domain/live/test
  auto list = base::StringUtils::SplitString(url, "/");
  if(list.size() < 5)
  {
    return "";
  }
  std::string domain, app, stream;
  if(list.size() == 5)
  {
    domain = list[2];
    app = list[3];
    stream = list[4];
  }
  else if(list.size() == 6)
  {
    domain = list[3];
    app = list[4];
    stream = list[5];
  }
  auto pos = domain.find_first_of(':');
  //如果有冒号，去掉冒号后面部分 
  if(pos != std::string::npos)
  {
    domain = domain.substr(0, pos);
  }
  return domain + "/" + app + "/" + stream;
}
