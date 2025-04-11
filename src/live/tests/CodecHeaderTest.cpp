#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "mmedia/rtmp/RtmpClient.h"
#include "live/CodecHeader.h"
#include "live/base/CodecUtils.h"
using namespace tmms::network;
using namespace tmms::mm;
using namespace tmms::live;


EventLoopThread eventloop_thread;
std::thread th;
CodecHeader codec_header;

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
    if(CodecUtils::IsCodecHeader(data))
    {
      codec_header.ParseCodecHeader(data);
    }
  }
  void OnRecv(const TcpConnectionPtr &conn ,PacketPtr &&data) override
  {
    if(CodecUtils::IsCodecHeader(data))
    {
      codec_header.ParseCodecHeader(data);
    }
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
