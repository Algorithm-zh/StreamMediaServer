#include "../Acceptor.h"
#include "../../base/Network.h"
#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include <iostream>
#include "../../base/InetAddress.h"
#include "../../UdpClient.h"
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;
const char *http_request = "GET / HTTP/1.0\r\nHost: 172.18.12.3\r\nAccept: */*\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
const char * http_response = "HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    InetAddress server("10.17.1.19:34444");
    std::shared_ptr<UdpClient> client = std::make_shared<UdpClient>(loop, server);
    client->SetRecvMsgCallback([](const InetAddress &addr, MsgBuffer &buf){
      std::cout << "host:" << addr.ToIpPort() << " recv msg:" << buf.Peek() << std::endl;
      buf.RetrieveAll();
    });
    client->SetWriteCompleteCallback([](const UdpSocketPtr &con){
      if(con)
      {
        std::cout << "write complete host:" << con->PeerAddr().ToIpPort() << std::endl;
      }
    });
    client->SetCloseCallback([](const UdpSocketPtr &con){
      if(con)
      {
        std::cout << "close host:" << con->PeerAddr().ToIpPort() << std::endl;
      }
    });
    client->SetConnectedCallback([&client](const UdpSocketPtr &con, bool connected){
      if(connected)
      {
        client->Send("1111", strlen("1111"));
      }
    });
    std::cout << "start connect" << std::endl;
    client->Connect();
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

  return 0;
}
