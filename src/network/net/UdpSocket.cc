#include "UdpSocket.h"
#include "../base/Network.h"
#include <algorithm>
#include <memory>
#include <sys/socket.h>
using namespace tmms::network;
 
UdpSocket::UdpSocket(EventLoop *loop, int socketfd, const InetAddress &localAddr, const InetAddress &peerAddr)  
:Connection(loop, socketfd, localAddr, peerAddr), message_buffer_(65535)
{

 
}
 
UdpSocket::~UdpSocket()  {
 
}
 
void UdpSocket::OnTimeout()  {
 
  NETWORK_TRACE << "host:" << peer_addr_.ToIpPort() << " timeout";
  OnClose();
}
 
void UdpSocket::OnError(const std::string &msg)  {
 
  NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " error:" << msg; 
  OnClose();
}
 
void UdpSocket::OnRead()  {
 
  if(closed_)
  {
    NETWORK_WARN << "host:" << peer_addr_.ToIpPort() << " had closed.";
    return;
  }
  ExtendLife();
  while(true)
  {
    struct sockaddr_in6 sock_addr;
    socklen_t len = sizeof(sock_addr);
    auto ret = ::recvfrom(fd_, 
                          message_buffer_.BeginWrite(), 
                          message_buffer_size_,
                          0, 
                          (struct sockaddr *)&sock_addr, 
                          &len);
    if(ret > 0)
    {
      InetAddress peeraddr;
      message_buffer_.HasWritten(ret);
      if(sock_addr.sin6_family == AF_INET6)
      {
        char ip[INET6_ADDRSTRLEN]{0};
        ::inet_ntop(AF_INET6, &sock_addr.sin6_addr, ip, sizeof(ip));
        peeraddr.SetAddr(ip);
        peeraddr.SetPort(ntohs(sock_addr.sin6_port));
        peeraddr.SetIsIPV6(true);
      }
      else
      {
        char ip[16]{0};
        struct sockaddr_in *addr = (struct sockaddr_in *)&addr;
        ::inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
        peeraddr.SetAddr(ip);
        peeraddr.SetPort(ntohs(addr->sin_port));
      }
      if(message_cb_)
      {
        message_cb_(peeraddr, message_buffer_);
      }
      //面向数据包，所以每次只处理一个数据，将剩余的数据清空
      message_buffer_.RetrieveAll();
    }
    else if(ret < 0)
    {
      if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
      {
        NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " recvfrom error:" << strerror(errno);
        OnClose();
        return;
      }
      break;
    }
  }
}
 
void UdpSocket::OnWrite()  {

 if(closed_)
  {
    NETWORK_WARN << "host:" << peer_addr_.ToIpPort() << " had closed.";
    return;
  }
  ExtendLife();
  while(true)
  {
    if(!buffer_list_.empty())
    {
      auto buf = buffer_list_.front();
      auto ret = ::sendto(fd_, buf->addr, buf->size, 0, (struct sockaddr *)&buf->sock_addr, buf->sock_len);
      if(ret > 0)
      {
        buffer_list_.pop_front();
      }
      else if(ret < 0)
      {
        if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
        {
          NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " sendto error:" << strerror(errno);
          OnClose();
          return;
        }
        break;
      }
    }
    if(buffer_list_.empty())
    {
      if(write_complete_cb_)
      {
        write_complete_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
      }
      break;
    }

  }
}
 
void UdpSocket::OnClose()  {
 
  if(!closed_)
  {
    closed_ = true;
    if(close_cb_)
    {
      close_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
    }
    Event::Close();
  }
}
 
void UdpSocket::SetCloseCallback(const UdpSocketCloseCallback &cb)  {
  close_cb_ = cb; 
}
 
void UdpSocket::SetCloseCallback(UdpSocketCloseCallback &&cb)  {
  close_cb_ = std::move(cb); 
}
 
void UdpSocket::SetRecvMsgCallback(const UdpSocketMessageCallback &cb)  {
  message_cb_ = cb; 
}
 
void UdpSocket::SetRecvMsgCallback(UdpSocketMessageCallback &&cb)  {
  message_cb_ = std::move(cb); 
}
 
void UdpSocket::SetWriteCompleteCallback(const UdpSocketWriteCompleteCallback &cb)  {
  write_complete_cb_ = cb;
}
 
void UdpSocket::SetWriteCompleteCallback(UdpSocketWriteCompleteCallback &&cb)  {
  write_complete_cb_ = std::move(cb); 
}
 
void UdpSocket::SetTimeoutCallback(int timeout, const UdpSocketTimeoutCallback &cb)  {
  auto us = std::dynamic_pointer_cast<UdpSocket>(shared_from_this());  
  loop_->RunAfter(timeout, [this, cb, us](){
    cb(us);
  });
}
 
void UdpSocket::SetTimeoutCallback(int timeout, UdpSocketTimeoutCallback &&cb)  {
 
}
 
void UdpSocket::ExtendLife()  {
  auto tp = timeout_entry_.lock(); 
  if(tp)
  {
    loop_->InsertEntry(max_idle_time_, tp);
  }
}
 
void UdpSocket::EnableCheckIdleTimeout(int32_t max_time)  {
 
  auto tp = std::make_shared<UdpTimeoutEntry>(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
  max_idle_time_ = max_time;
  timeout_entry_ = tp;
  loop_->InsertEntry(max_idle_time_, tp);
}
 
void UdpSocket::Send(std::list<UdpBufferNodePtr> &list)  {
  loop_->RunInLoop([this, &list](){
    SendInLoop(list);
  });
}
 
void UdpSocket::Send(const char *buf, size_t size, struct sockaddr *addr, socklen_t len)  {
  loop_->RunInLoop([this, buf, size, addr, len](){
    SendInLoop(buf, size, addr, len);
  });
}
 
void UdpSocket::SendInLoop(std::list<UdpBufferNodePtr> &list)  {
 
  for(auto &l : list)
  {
    buffer_list_.push_back(l);
  }
  if(!buffer_list_.empty())
  {
    EnableWriting(true);
  }
}
 
void UdpSocket::SendInLoop(const char *buf, size_t size, struct sockaddr *addr, socklen_t len)  {
  //为空就单独发送 
  if(buffer_list_.empty())
  {
    auto ret = ::sendto(fd_, buf, size, 0, addr, len);
    if(ret > 0)
    {
        return;
    }
  }
  //否则就放到队列里
  auto node = std::make_shared<UdpBufferNode>(const_cast<char *>(buf), size, addr, len);
  buffer_list_.push_back(node);
  EnableWriting(true);
}
 
void UdpSocket::ForceClose()  {
  loop_->RunInLoop([this](){
    OnClose();
  });
}
