
#include "../../network/net/Acceptor.h"
#include "../../network/base/Network.h"
#include "../../network/net/EventLoop.h"
#include "../../network/net/EventLoopThread.h"
#include "../../network/base/InetAddress.h"
#include "../../network/TcpClient.h"
#include <chrono>
#include <thread>
#include "../rtmp/RtmpClient.h"
using namespace tmms::network;
using namespace tmms::mm;

EventLoopThread eventloop_thread;
std::thread th;

class RtmpHandlerImpl : public RtmpHandler
{
public:
  void OnNewConnection(const TcpConnectionPtr &conn) override
  {

  }
  void OnConnectionDestroy(const TcpConnectionPtr &conn) override
  {

  }
  void OnRecv(const TcpConnectionPtr &conn ,const PacketPtr &data) override
  {
    std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
  }
  void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override
  {
    std::cout << "recv type:" << data->PacketType() << " size:" << data->PacketSize() << std::endl;
  }
  void OnActive(const ConnectionPtr &conn) override
  {
    
  }
  bool OnPlay(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override  
  {
    return false;
  }
  bool OnPublish(const TcpConnectionPtr &conn, const std::string &session_name, const std::string &param) override
  {
    return false;
  }
  void OnPause(const TcpConnectionPtr &conn, bool pause) override
  {
  }
  void OnSeek(const TcpConnectionPtr &conn, double time) override
  {
  }
};
int main(int argc, char *argv[]) {

  eventloop_thread.Run();
  EventLoop *loop = eventloop_thread.Loop();

  if(loop)
  {
    RtmpClient client(loop, new RtmpHandlerImpl());
    client.Play("rtmp://localhost/live/test");
    while(1)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  return 0;
}
