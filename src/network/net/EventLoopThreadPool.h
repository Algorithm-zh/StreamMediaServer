#pragma once
#include "../../base/NonCopyable.h"
#include <unistd.h>
#include <vector>
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <atomic>
namespace tmms
{
  namespace network
  {
    using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;
    class EventLoopThreadPool : public base::NonCopyable
    {
    public:
      EventLoopThreadPool(size_t thread_num, int start = 0, int cpus = 4);
      ~EventLoopThreadPool();
      //返回所有的事件循环
      std::vector<EventLoop*> GetLoops() const;
      EventLoop *GetNextLoop();
      size_t Size();
      void Start();
    private:
      std::vector<EventLoopThreadPtr> threads_;
      std::atomic_int32_t loop_index_{0};//当前取的哪个loop
    };
  }
}
