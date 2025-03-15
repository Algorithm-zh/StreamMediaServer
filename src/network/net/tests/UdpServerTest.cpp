#include "../Acceptor.h"
#include "../../base/Network.h"
#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include <iostream>
#include <memory>
#include "../../base/InetAddress.h"
#include "../..//UdpServer.h"
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    InetAddress listen("10.17.1.19:34444");
    std::shared_ptr<UdpServer> server = std::make_shared<UdpServer>(loop, listen);
    server->SetRecvMsgCallback([&server](const InetAddress &addr, MsgBuffer &buf){
      std::cout << "host:" << addr.ToIpPort() << " recv msg:" << buf.Peek() << std::endl;
      struct sockaddr_in6 sock_addr;
      addr.GetSockAddr((struct sockaddr *)&sock_addr);
      server->Send(buf.Peek(), buf.ReadableBytes(), (struct sockaddr*)&sock_addr, sizeof(sock_addr));
      buf.RetrieveAll();
    });
    server->SetWriteCompleteCallback([](const UdpSocketPtr &con){
      if(con)
      {
        std::cout << "write complete host:" << con->PeerAddr().ToIpPort() << std::endl;
      }
    });
    server->SetCloseCallback([](const UdpSocketPtr &con){
      if(con)
      {
        std::cout << "close host:" << con->PeerAddr().ToIpPort() << std::endl;
      }
    });
    server->Start();
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

  return 0;
}
