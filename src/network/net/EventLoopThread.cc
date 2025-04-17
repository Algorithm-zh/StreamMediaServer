#include "EventLoopThread.h"
using namespace tmms::network;
 
EventLoopThread::EventLoopThread()  
:thread_([this](){StartEventLoop();}){
 
}
 
EventLoopThread::~EventLoopThread()  {
  Run();
  if(loop_)
  {
    loop_->Quit();
  }
  if(thread_.joinable())
  {
    thread_.join();
  }
}
 
void EventLoopThread::Run()  {
  
  //只运行一次
  std::call_once(once_, [this](){
    {
      std::lock_guard<std::mutex> lk(lock_);
      running_ = true;
      condition_.notify_all();
    }
    //保证事件循环和事件循环线程同步
    auto f = promise_loop_.get_future();
    //持续等待有值才返回
    f.get();
  });
}
 
EventLoop* EventLoopThread::Loop() const
{
  return loop_;
}
 
void EventLoopThread::StartEventLoop()  {
  //用局部变量保证事件循环和事件循环线程同步
  EventLoop loop; 
  std::unique_lock<std::mutex> lk(lock_);
  condition_.wait(lk, [this]{return running_;});
  loop_ = &loop;
  promise_loop_.set_value(1);
  loop.Loop();
  loop_ = nullptr;
}
 
std::thread &EventLoopThread::Thread()  {
	return thread_;
}
