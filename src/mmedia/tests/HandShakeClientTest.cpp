
#include "../../network/net/Acceptor.h"
#include "../../network/base/Network.h"
#include "../../network/net/EventLoop.h"
#include "../../network/net/EventLoopThread.h"
#include "../../network/base/InetAddress.h"
#include "../../network/TcpClient.h"
#include "../rtmp/RtmpHandShake.h"
using namespace tmms::network;
using namespace tmms::mm;

using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;
EventLoopThread eventloop_thread;
std::thread th;
const char *http_request = "GET / HTTP/1.0\r\nHost: 10.17.1.19\r\nAccept: */*\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
const char * http_response = "HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    InetAddress server("10.17.1.19:1935");
    std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(loop, server);
    client->SetRecvMsgCallback([](const TcpConnectionPtr& con, MsgBuffer &buf){
      RtmpHandShakePtr shake = con->GetContext<RtmpHandShake>(kNormalContext);
      shake->HandShake(buf);
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
        RtmpHandShakePtr shake = con->GetContext<RtmpHandShake>(kNormalContext);
        shake->WriteComplete();
      }
    });
    client->SetConnectCallback([](const TcpConnectionPtr& con, bool connected){
      if(connected)
      {
        RtmpHandShakePtr shake = std::make_shared<RtmpHandShake>(con, true);
        con->SetContext(kNormalContext, shake);
        shake->Start();
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
