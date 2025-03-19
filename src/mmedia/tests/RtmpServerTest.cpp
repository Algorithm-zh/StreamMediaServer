#include "../../network/net/EventLoop.h"
#include "../../network/net/EventLoopThread.h"
#include "../../network/base/InetAddress.h"
#include "../rtmp/RtmpServer.h"
#include "../../network/base/MsgBuffer.h"
#include "../rtmp/RtmpHandShake.h"
#include <iostream>
#include <memory>
using namespace tmms::network;
using namespace tmms::mm;

EventLoopThread eventloop_thread;
std::thread th;
using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;
const char * http_response = "HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    InetAddress listen("10.17.1.19:1935");
    RtmpServer server(loop, listen);
    server.Start();
    while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

  return 0;
}
