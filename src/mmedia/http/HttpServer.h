#include "HttpHandler.h"
#include "../../network/base/InetAddress.h"
#include "../../network/TcpServer.h"
#include <memory>

namespace tmms
{
  namespace mm
  {
    using namespace tmms::network;
    class HttpServer : public TcpServer
    {
    public:
      //构造函数
      HttpServer(EventLoop *loop, const InetAddress &local, HttpHandler *handler = nullptr);
      ~HttpServer();
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

      HttpHandler *http_handler_{nullptr};
    };
  }
}
