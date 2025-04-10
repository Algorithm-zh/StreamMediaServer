#include "../EventLoopThread.h"
#include "../EventLoop.h"
#include "../PipeEvent.h"
#include <chrono>
#include <cstdint>
#include <thread>
#include "../EventLoopThreadPool.h"
#include "../EventLoopThread.h"
#include "../../../base/TTime.h"
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;  

void TestEventLoopThread() {
  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();
  if(loop)
  {
    std::cout << "loop: " << loop << std::endl;
    PipeEventPtr pipe = std::make_shared<PipeEvent>(loop);
    loop->AddEvent(pipe);
    int64_t test = 12345;
    th = std::thread([&pipe](){
      while(true)
      {
        int64_t now = tmms::base::TTime::NowMs();
        pipe->Write((const char*)&now, sizeof(now));
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    });
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
}

void TestEventLoopThreadPool() {
  
  EventLoopThreadPool pool(2, 0, 2);
  pool.Start();
  std::cout << "thread id: " << std::this_thread::get_id() << std::endl;
  std::vector<EventLoop*> list = pool.GetLoops();
  for(auto &loop : list)
  {
    loop->RunInLoop([&loop](){
      std::cout << "loop: " << loop << " thread id: " << std::this_thread::get_id() << std::endl;
    });
  }
}

void TestTimingWheel() {

  EventLoopThreadPool pool(2, 0, 2);
  pool.Start();
  EventLoop *loop = pool.GetNextLoop();
  std::cout << "loop: " << loop << " thread id: " << std::this_thread::get_id() << std::endl;
  loop->RunAfter(1, [](){
    std::cout << "run after 1000 ms:" << tmms::base::TTime::Now() << std::endl;
  });
  loop->RunAfter(5, [](){
    std::cout << "run after 5000 ms:" << tmms::base::TTime::Now() << std::endl;
  });
  loop->RunEvery(1, [](){
    std::cout << "run every 1000 ms:" << tmms::base::TTime::Now() << std::endl;
  });
  loop->RunEvery(5, [](){
    std::cout << "run every 5000 ms:" << tmms::base::TTime::Now() << std::endl;
  });
  while(1)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
int main() {
  TestTimingWheel();
  while(true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return 0;
}
