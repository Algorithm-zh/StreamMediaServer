#include "../Acceptor.h"
#include "../../base/Network.h"
#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include "../../base/InetAddress.h"
#include "../../TcpClient.h"
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;
const char *http_request = "GET / HTTP/1.0\r\nHost: 10.17.1.19\r\nAccept: */*\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
const char * http_response = "HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    InetAddress server("10.17.1.19:34444");
    std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(loop, server);
    client->SetRecvMsgCallback([](const TcpConnectionPtr& con, MsgBuffer &buf){
      std::cout << "host: " << con->PeerAddr().ToIpPort() << " recv msg:" << buf.Peek() << std::endl;
      buf.RetrieveAll();
    });
    client->SetCloseCallback([](const TcpConnectionPtr& con){
      if(con)
      {
        std::cout << "host:" << con->PeerAddr().ToIpPort() << " close" << std::endl;
      }
    });
    client->SetWriteCompleteCallback([](const TcpConnectionPtr& con){
      if(con)
      {
        std::cout << "host:" << con->PeerAddr().ToIpPort() << " write complete" << std::endl;
      }
    });
    client->SetConnectCallback([](const TcpConnectionPtr& con, bool connected){
      if(connected)
      {
        std::list<BufferNodePtr> list;
        auto size = htonl(strlen(http_request));
        BufferNodePtr node = std::make_shared<BufferNode>((void *)&size, sizeof(size));
        list.emplace_back(std::move(node));
        //con->Send((const char*)&size, sizeof(size));
        con->Send(list);
        con->Send(http_request, strlen(http_request));
      }
    });
    client->Connect();
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

  return 0;
}
