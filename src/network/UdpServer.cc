#include "UdpServer.h"
#include "base/SocketOpt.h"
#include "net/UdpSocket.h"
#include <memory>
using namespace tmms::network;
 
UdpServer::UdpServer(EventLoop *loop, const InetAddress &server)
:UdpSocket(loop, -1, server, InetAddress()), server_(server) {
 
}
 
UdpServer::~UdpServer()  {
  Stop();
}
 
void UdpServer::Start()  {
  loop_->RunInLoop([this](){
    Open();
  });
}
 
void UdpServer::Stop()  {
  loop_->RunInLoop([this](){
    loop_->DelEvent(std::dynamic_pointer_cast<UdpServer>(shared_from_this())); 
    OnClose();
  });
}
 
void UdpServer::Open()  {
  loop_->AssertInLoopThread();
  fd_ = SocketOpt::CreateNonblockingUdpSocket(AF_INET);
  if(fd_ < 0)
  {
    NETWORK_ERROR << "create udp socket error";
    OnClose();
    return;
  }
  loop_->AddEvent(std::dynamic_pointer_cast<UdpServer>(shared_from_this()));
  SocketOpt opt(fd_);
  opt.BindAddress(server_);
}
