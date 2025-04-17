#include "PipeEvent.h"
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include "../base/Network.h"
#include <iostream>
using namespace tmms::network;
 
PipeEvent::PipeEvent(EventLoop *loop)  
:Event(loop)  {
  int fd[2] = {0,}; 
  auto ret = ::pipe2(fd, O_NONBLOCK);
  if(ret < 0)
  {
    NETWORK_ERROR << "pipe2 error: " << strerror(errno);
    exit(1);
  }
  fd_ = fd[0];//fd[0]读
  write_fd_ = fd[1];//fd[1]写
}
 
PipeEvent::~PipeEvent()  {
  if(write_fd_ > 0) 
  {
    ::close(write_fd_);
    write_fd_ = -1;
  }
}
 
void PipeEvent::OnRead()  {
 
  int64_t tmp;
  auto ret = ::read(fd_, &tmp, sizeof(tmp));
  if(ret < 0)
  {
    NETWORK_ERROR << "read error: " << strerror(errno);
    return;
  }
  
}
 
 
void PipeEvent::OnError(const std::string &err_msg)  {
   
  std::cout << "pipe error: " << err_msg << std::endl;
}
 
void PipeEvent::Write(const char*data, size_t len)  {
   
  ::write(write_fd_, data, len);
}
 
void PipeEvent::OnClose()  {
 
  if(write_fd_ > 0) 
  {
    ::close(write_fd_);
    write_fd_ = -1;
  }
}
