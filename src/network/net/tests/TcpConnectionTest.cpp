#include "../Acceptor.h"
#include "../../base/Network.h"
#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include <iostream>
#include <memory>
#include <vector>
#include "../../base/InetAddress.h"
#include "../TcpConnection.h"
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;

const char * http_response = "HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    std::vector<TcpConnectionPtr> list;
    InetAddress server("10.17.1.19:34444");
    std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop, server);
    acceptor->SetAcceptCallback([&loop,&server,&list](int sock, const InetAddress &addr){
      std::cout << "host:" << addr.ToIpPort() << std::endl;
      TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop, sock, server, addr);
      conn->SetRecvMsgCallback([](const TcpConnectionPtr &conn, MsgBuffer &buffer){
        std::cout << "recv msg:" << buffer.Peek() << std::endl;
        buffer.RetrieveAll();
        //conn->Send(http_response, strlen(http_response));
      });
      conn->SetWriteCompleteCallback([&loop](const TcpConnectionPtr &conn){
        std::cout << "write complete host:" << conn->PeerAddr().ToIpPort() << std::endl;
        loop->DelEvent(conn);
        conn->ForceClose();
      });
      conn->SetTimeoutCallback(1000, [](const TcpConnectionPtr &conn){
        std::cout << "timeout host:" << conn->PeerAddr().ToIpPort() << std::endl;
        conn->ForceClose();
      });
      conn->EnableCheckIdleTimeout(3);
      list.push_back(conn);
      loop->AddEvent(conn);
    });
    acceptor->Start();
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

  return 0;
}
