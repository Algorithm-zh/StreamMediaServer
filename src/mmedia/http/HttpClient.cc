#include "HttpClient.h"
#include "HttpContext.h"
#include "mmedia/base/MMediaLog.h"
#include "network/DnsService.h"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
using namespace tmms::mm;
using namespace tmms::network;
 
HttpClient::HttpClient(EventLoop *loop, HttpHandler *handler)
:loop_(loop), handler_(handler){
 
}
 
HttpClient::~HttpClient(){
 
}
 
void HttpClient::OnWriteComplete(const TcpConnectionPtr & conn)  {
 
  auto context = conn->GetContext<HttpContext>(kHttpContext); 
  if(context)
  {
    context->WriteComplete(conn);
  }
}
 
void HttpClient::OnConnection(const TcpConnectionPtr & conn, bool connected)  {
 
  if(connected)
  {
    auto context = std::make_shared<HttpContext>(conn, handler_);
    conn->SetContext(kHttpContext, context);
    if(is_post_)
    {
      context->PostRequest(request_->MakeHeaders(), out_packet_);
    }
    else
    {
      context->PostRequest(request_->MakeHeaders());
    }
  }
}
 
void HttpClient::OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf)  {
  auto context = conn->GetContext<HttpContext>(kHttpContext); 
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
 
void HttpClient::SetCloseCallback(const CloseConnectionCallback &cb)  {
  close_cb_ = cb; 
}
 
void HttpClient::SetCloseCallback(CloseConnectionCallback && cb)  {
  close_cb_ = std::move(cb); 
}
 

 
bool HttpClient::ParseUrl(const std::string &url)  {

  if(url.size() > 7)//http://domain
  {
    uint16_t port = 80;
    auto pos = url.find_first_of("/", 7);
    if(pos != std::string::npos)
    {
      const std::string &path = url.substr(pos);
      auto pos1 = path.find_first_of("?");
      if(pos1 != std::string::npos)
      {
        request_->SetPath(path.substr(0, pos1));
        request_->SetQuery(path.substr(pos1 + 1));
      }
      else
      {
        request_->SetPath(path);
      }
      std::string domain = url.substr(7, pos - 7);
      request_->AddHeader("Host", domain);
      auto pos2 = domain.find_first_of(":");
      if(pos2 != std::string::npos)
      {
        addr_.SetAddr(domain.substr(0, pos2));
        addr_.SetPort(std::atoi(url.substr(pos2 + 1).c_str()));
      }
      else
      {
        addr_.SetAddr(domain);
        addr_.SetPort(port);
      }
    }
    else
    {
      request_->SetPath("/");
      std::string domain = url.substr(7);
      request_->AddHeader("Host",domain);
      addr_.SetAddr(domain);
      addr_.SetPort(port);
    }
    //获取域名ip
    //直接从表里拿
    auto list = sDnsService->GetHostAddress(addr_.IP());
    if(list.size() > 0)
    {
      for(auto const &l : list)
      {
        if(!l->IsIpV6())
        {
          addr_.SetAddr(l->IP());
          break;
        }
      }
    }
    else
    {
      //之前没有解析过这个域名那就先解析
      sDnsService->AddHost(addr_.IP());//加到列表里下次就可以直接取
      std::vector<InetAddressPtr> list2;
      sDnsService->GetHostInfo(addr_.IP(), list2);
      if(list2.size() > 0)
      {
        for(auto const &l : list2)
        {
          if(!l->IsIpV6())
          {
            addr_.SetAddr(l->IP());
            break;
          }
        }
      }
    }
    return true;
  }
  
  return false;
}
 
void HttpClient::CreateTcpClient()  {
 
  request_.reset();
  request_ = std::make_shared<HttpRequest>(true);
  if(is_post_)
  {
    request_->SetMethod(HttpMethod::kPost);
  }
  else
  {
    request_->SetMethod(HttpMethod::kGet);
  }
  request_->AddHeader("User-Agent", "curl/7.61.1");
  request_->AddHeader("Accept", "*/*");
  auto ret = ParseUrl(url_);
  if(!ret)
  {
    RTMP_ERROR << "invalid url: " << url_;
    if(close_cb_)
    {
      close_cb_(nullptr);
    }
    return ;
  }
  tcp_client_ = std::make_shared<TcpClient>(loop_, addr_);
  tcp_client_->SetWriteCompleteCallback(std::bind(&HttpClient::OnWriteComplete, this, std::placeholders::_1));
  tcp_client_->SetRecvMsgCallback(std::bind(&HttpClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
  tcp_client_->SetCloseCallback(close_cb_);
  tcp_client_->SetConnectCallback(std::bind(&HttpClient::OnConnection, this, std::placeholders::_1, std::placeholders::_2));
  tcp_client_->Connect();
}
 
void HttpClient::Get(const std::string &url)  {
  is_post_ = false; 
  url_ = url;
  CreateTcpClient();
}
 
void HttpClient::Post(const std::string &url, const PacketPtr &packet)  {
  is_post_ = true; 
  url_ = url;
  out_packet_ = packet;
  CreateTcpClient();
}
