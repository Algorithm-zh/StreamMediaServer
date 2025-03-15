#include "TcpClient.h"
#include "base/Network.h"
#include "base/SocketOpt.h"
using namespace tmms::network;

 
TcpClient::TcpClient(EventLoop *loop, const InetAddress &server)
:TcpConnection(loop, -1, InetAddress(), server), server_addr_(server), status_(kTcpConStatusInit)
{
 
}
 
TcpClient::~TcpClient()  {
  OnClose();
}
 
void TcpClient::Connect()  {
  loop_->RunInLoop([this](){
    ConnectInLoop();
  }); 
}
 
void TcpClient::ConnectInLoop()  {
  loop_->AssertInLoopThread();
  fd_ = SocketOpt::CreateNonblockingTcpSocket(AF_INET);
  if(fd_ < 0)
  {
    OnClose();
    return;
  }
  status_ = kTcpConStatusConnecting;
  loop_->AddEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
  EnableWriting(true);
  EnableCheckIdleTimeout(3);
  SocketOpt opt(fd_);
  auto ret = opt.Connect(server_addr_);
  if(ret == 0)
  {
    UpdateConnectionStatus();
    return;
  }
  else if(ret == -1)
  {
    if(errno != EINPROGRESS)
    {
      NETWORK_ERROR << "connect to server:" << server_addr_.ToIpPort() << " error:" << strerror(errno);
      OnClose();
      return;
    }
  }
}
 
void TcpClient::SetConnectCallback(const ConnectionCallback &cb)  {
  connected_cb_ = cb;
}
 
void TcpClient::SetConnectCallback(ConnectionCallback &&cb)  {
  connected_cb_ = std::move(cb);
}
 
void TcpClient::UpdateConnectionStatus()  {
  status_ = kTcpConStatusConnected;
  if(connected_cb_)
  {
    connected_cb_(std::dynamic_pointer_cast<TcpClient>(shared_from_this()), true);
  }
}
 
bool TcpClient::CheckError()  {
  int error = 0;
  socklen_t len = sizeof(error);
  ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &error, &len);
  return error != 0;
}
 
void TcpClient::OnRead()  {
 
  if(status_ == kTcpConStatusConnecting)
  {
    if(CheckError())
    {
      NETWORK_ERROR << "connect to server:" << server_addr_.ToIpPort() << " error:" << strerror(errno);
      OnClose();
      return;
    }
    UpdateConnectionStatus();
    return ;
  }
  else if(status_ == kTcpConStatusConnected)
  {
    TcpConnection::OnRead();
  }
}
 
void TcpClient::OnWrite()  {
 
 
  if(status_ == kTcpConStatusConnecting)
  {
    if(CheckError())
    {
      NETWORK_ERROR << "connect to server:" << server_addr_.ToIpPort() << " error:" << strerror(errno);
      OnClose();
      return;
    }
    UpdateConnectionStatus();
    return ;
  }
  else if(status_ == kTcpConStatusConnected)
  {
    TcpConnection::OnWrite();
  }
}
 
void TcpClient::OnClose()  {
 
  if(status_ == kTcpConStatusConnecting || status_ == kTcpConStatusConnected)
  {
    loop_->DelEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
  }
  status_ = kTcpConStatusDisconnected;
  TcpConnection::OnClose();
}

void TcpClient::Send(std::list <BufferNodePtr> &list)  {
 
  if(status_ == kTcpConStatusConnected)
  {
    TcpConnection::Send(list);
  }
}

 
void TcpClient::Send(const char *buf, size_t size)  {
   
  if(status_ == kTcpConStatusConnected)
  {
    TcpConnection::Send(buf, size);
  }
}
