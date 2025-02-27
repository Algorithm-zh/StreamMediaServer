#pragma once
#include <string>
#include <sys/epoll.h>
#include <memory>

namespace tmms 
{
  namespace network 
  {
    class EventLoop;
    //设置边沿模式，高优先级
    const int kEventRead = (EPOLLIN|EPOLLPRI|EPOLLET);
    const int kEventWrite = (EPOLLOUT|EPOLLET);
    class Event : public std::enable_shared_from_this<Event>
    {
      friend class EventLoop;
    public:
      Event();
      Event(EventLoop *loop, int fd);
      ~Event();
      virtual void OnRead(){};
      virtual void OnWrite(){};
      virtual void OnError(const std::string &err_msg){};
      virtual void OnClose(){};
      bool EnableWriting(bool enable);
      bool EnableReading(bool enable);
      int Fd() const; 
    private:
      EventLoop *loop_{nullptr};
      int fd_{-1};
      int event_{0};
    };
  }
}
