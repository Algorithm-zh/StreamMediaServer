#include "../Acceptor.h"
#include "../../base/Network.h"
#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include <iostream>
#include <memory>
#include "../../base/InetAddress.h"
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop, InetAddress("10.17.1.19:34444"));
    acceptor->SetAcceptCallback([](int sock, const InetAddress &addr){
      std::cout << "host:" << addr.ToIpPort() << std::endl;
    });
    acceptor->Start();
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

  return 0;
}
