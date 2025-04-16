#pragma once 
#include "base/NonCopyable.h"
#include "EventLoop.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

namespace tmms
{
    namespace network
    {
        class EventLoopThread:public base::NonCopyable
        {
        public:
            EventLoopThread();
            ~EventLoopThread();

            void Run();
            
            EventLoop *Loop() const;
            std::thread &Thread() ;
        private:
            void StartEventLoop();

            EventLoop * loop_{nullptr};
            bool running_{false};
            std::mutex lock_;
            std::condition_variable condition_;
            std::once_flag once_;
            std::promise<int> promise_loop_;
            std::thread thread_;
        };
    }
}