#include "RtmpHandler.h"
#include "../../network/base/InetAddress.h"
#include "../../network/TcpServer.h"
#include "RtmpHandShake.h"

namespace tmms
{
  namespace mm
  {
    using namespace tmms::network;
    class RtmpServer : public TcpServer
    {
    public:
      //构造函数
      RtmpServer(EventLoop *loop, const InetAddress &local, RtmpHandler *handler = nullptr);
      ~RtmpServer();
      //启动和停止函数
      void Start() override;
      void Stop() override;
    private:
      //内部回调成员函数
      void OnNewConnection(const TcpConnectionPtr &conn);
      void OnDestroyed(const TcpConnectionPtr &conn);
      void OnMessage(const TcpConnectionPtr &conn, MsgBuffer &buf);
      void OnWriteComplete(const ConnectionPtr &conn);
      void OnActive(const ConnectionPtr &conn);

      RtmpHandler *rtmp_handler_{nullptr};
    };
    using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;
  }
}
