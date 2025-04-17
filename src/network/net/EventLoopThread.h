#pragma once
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../../base/NonCopyable.h"
#include "EventLoop.h"

namespace tmms
{
  namespace network
  {
    class EventLoopThread : public base::NonCopyable
    {
    public:
      EventLoopThread();
      ~EventLoopThread();
      void Run();
      EventLoop *Loop() const;
      std::thread &Thread();
    private:
      void StartEventLoop();

      std::mutex lock_;
      std::condition_variable condition_;
      EventLoop* loop_{nullptr};
      bool running_{false};
      std::once_flag once_;
      std::promise<int> promise_loop;
      std::thread thread_;
      
    };
  }
}
