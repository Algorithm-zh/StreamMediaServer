#include "../Acceptor.h"
#include "../../base/Network.h"
#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include <iostream>
#include <memory>
#include "../../base/InetAddress.h"
#include "../../TcpServer.h"
#include "../../TestContext.h"
using namespace tmms::network;

EventLoopThread eventloop_thread;
std::thread th;
using TestContextPtr = std::shared_ptr<TestContext>;
const char * http_response = "HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    InetAddress listen("10.17.1.19:34444");
    TcpServer server(loop, listen);
    server.SetMessageCallback([](const TcpConnectionPtr& con, MsgBuffer &buf){
      TestContextPtr context = con->GetContext<TestContext>(kNormalContext);
      context->ParseMessage(buf);
    });
    server.SetNewConnectionCallback([&loop](const TcpConnectionPtr& con){
      TestContextPtr context = std::make_shared<TestContext>(con);
      context->SetTestMessageCallback([&loop](const TcpConnectionPtr& con, const std::string &message){
        std::cout << "message: " << message << std::endl;
      });
      con->SetContext(kNormalContext, context);
      con->SetWriteCompleteCallback([&loop](const TcpConnectionPtr &con){
        std::cout << "write complete host:" << con->PeerAddr().ToIpPort() << std::endl;
        //con->ForceClose();
      });
    });
    server.Start();

    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

  return 0;
}
