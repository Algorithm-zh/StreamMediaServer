#include "TcpConnection.h"
#include <algorithm>
#include <cerrno>
#include <memory>
#include <unistd.h>
#include "../base/Network.h"
using namespace tmms::network;
 
TcpConnection::TcpConnection(EventLoop *loop, int socketfd, const InetAddress &localAddr, const InetAddress &peerAddr)
:Connection(loop, socketfd, localAddr, peerAddr){
 
}
 
TcpConnection::~TcpConnection()  {
  OnClose(); 
}
 
void TcpConnection::OnClose()  {
  loop_->AssertInLoopThread();
  if(!closed_)
  {
    closed_ = true;
    if(close_cb_)
    {
      close_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
    }
    //关闭文件描述符
    Event::Close();
  }
}
 
//有可能是其他线程去关闭
void TcpConnection::ForceClose()  {
  loop_->RunInLoop([this](){
    OnClose();
  });
}
 
void TcpConnection::SetCloseCallback(const CloseConnectionCallback &cb)  {
  close_cb_ = cb;
}
 
void TcpConnection::SetCloseCallback(CloseConnectionCallback &&cb)  {
  close_cb_ = std::move(cb);
}
 
void TcpConnection::OnRead()  {
  if(closed_) 
  {
    NETWORK_TRACE << "host:" << peer_addr_.ToIpPort() << " closed";
    return;
  }
  ExtendLife();
  while(true)
  {
    int err = 0;
    auto ret = message_buffer_.ReadFd(fd_, &err);
    if(ret > 0)
    {
      if(message_cb_)
      {
        message_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()), message_buffer_);
      }
    }else if(ret == 0)
    {
      OnClose();
      break;
    }
    else{
      if(err != EINTR && err != EAGAIN && err != EWOULDBLOCK)
      {
        NETWORK_ERROR << "read error" << strerror(err);
        OnClose();
      }
      break;
    }
  }
}
 
void TcpConnection::SetRecvMsgCallback(const MessageCallback &cb)  {
  message_cb_ = cb;
}
 
void TcpConnection::SetRecvMsgCallback(MessageCallback &&cb)  {
  message_cb_ = std::move(cb);
}
 
void TcpConnection::OnError(const std::string &msg)  {
  NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " error:" << msg;
  OnClose();
}
 
void TcpConnection::OnWrite()  {
  if(closed_) 
  {
    NETWORK_TRACE << "host:" << peer_addr_.ToIpPort() << " closed";
    return;
  }
  ExtendLife();
  if(!io_vec_list_.empty()) 
  {
    while(true)
    {
      //集中发送， 指定起始地址
      auto ret = ::writev(fd_, &io_vec_list_[0], io_vec_list_.size());
      auto s = io_vec_list_.front().iov_base;
      auto a = io_vec_list_.front().iov_len;
      if(ret >= 0)
      {
        while(ret > 0)
        {
          //说明第一个缓冲区没有发送完整
          if(io_vec_list_.front().iov_len > ret)
          {
            io_vec_list_.front().iov_base = (char *)io_vec_list_.front().iov_base + ret;
            io_vec_list_.front().iov_len -= ret;
            break;
          }
          else
          {
            ret -= io_vec_list_.front().iov_len;
            io_vec_list_.erase(io_vec_list_.begin());
          }
        }
        if(io_vec_list_.empty())
        {
          //没数据了，不要写
          EnableWriting(false);
          if(write_complete_cb_) 
          {
            write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
          }
          return;
        }
      }
      else
      {
        if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
        {
          NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " write error:" << strerror(errno);
          OnClose();
          return;
        }
        break;
      }
    }
  }
  else{
    EnableWriting(false);
    if(write_complete_cb_) 
    {
      write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
    }
  }

}
 
void TcpConnection::SetWriteCompleteCallback(const WriteCompleteCallback &cb)  {
  write_complete_cb_ = cb; 
}
 
void TcpConnection::SetWriteCompleteCallback(WriteCompleteCallback &&cb)  {
  write_complete_cb_ = std::move(cb); 
}

void TcpConnection::Send(std::list<BufferNodePtr> &list)  {
  loop_->RunInLoop([this, &list](){
    SendInLoop(list);
  });
}
 
void TcpConnection::Send(const char *buf, size_t size)  {
 
  loop_->RunInLoop([this, buf, size](){
    SendInLoop(buf, size);
  });
}
 
void TcpConnection::SendInLoop(std::list<BufferNodePtr> &list)  {
  if(closed_)
  {
    NETWORK_TRACE << "host:" << peer_addr_.ToIpPort() << "had closed.";
    return;
  }
  //添加到队列中待发送
  for(auto &l : list)
  {
    struct iovec vec;
    vec.iov_base = (void *)l->addr;
    vec.iov_len = l->size;

    io_vec_list_.push_back(vec);
  }
  if(!io_vec_list_.empty())
  {
    EnableWriting(true);
  }
}
 
void TcpConnection::SendInLoop(const char *buf, size_t size)  {
  if(closed_)
  {
    NETWORK_TRACE << "host:" << peer_addr_.ToIpPort() << "had closed.";
    return;
  }
  size_t send_len = 0;
  //
  if(io_vec_list_.empty())
  {
    send_len = ::write(fd_, buf, size);
    if(send_len < 0)
    {
        if(errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
        {
          NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " write error:" << strerror(errno);
          OnClose();
          return;
        }
        send_len = 0;
    }
    size -= send_len;
    //如果发送完整了，则回调
    if(size == 0)
    {
      if(write_complete_cb_)
      {
        write_complete_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
      }
      return;
    }
  }
  if(size > 0)
  {
    struct iovec vec;
    vec.iov_base = (char *)buf + send_len;
    vec.iov_len = size;

    io_vec_list_.push_back(vec);
    EnableWriting(true);
  }
}
 
 
void TcpConnection::OnTimeout()  {
  NETWORK_ERROR << "host:" << peer_addr_.ToIpPort() << " timeout";
  std::cout << "timeout host:" << peer_addr_.ToIpPort() << std::endl;
  OnClose();
}
 
void TcpConnection::SetTimeoutCallback(int timeout, const TimeoutCallback &cb)  {

  auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
  loop_->RunAfter(timeout, [&cp, cb](){
    cb(cp);
  });
}
 
void TcpConnection::SetTimeoutCallback(int timeout, TimeoutCallback &&cb)  {
  auto cp = std::dynamic_pointer_cast<TcpConnection>(shared_from_this());
  loop_->RunAfter(timeout, [&cp, cb](){
    cb(cp);
  });
}

void TcpConnection::ExtendLife()  {
  // 通过增加TimoutEntry的引用计数来增加生命周期，有引用Entry就不会被销毁
  auto tp = timeout_entry_.lock();
  if(tp)
  {
    //因为经过一个InseryEntry,智能指针会被销毁一个，等全部引用这个内存的智能指针全部消失Entry也会被析构
    loop_->InsertEntry(max_idle_time_, tp);
  }
}
 
void TcpConnection::EnableCheckIdleTimeout(int32_t max_time)  {
  
  auto tp = std::make_shared<TimeoutEntry>(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
  max_idle_time_ = max_time;
  timeout_entry_ = tp;
  loop_->InsertEntry(max_time, tp);
}
 

