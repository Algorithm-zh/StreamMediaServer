#pragma once
#include "mmedia/base/MMediaHandler.h"

namespace tmms
{
  namespace mm
  {
    class HttpRequest;
    using HttpRequestPtr = std::shared_ptr<HttpRequest>;
    class HttpHandler : public MMediaHandler
    {
    public:
        virtual void OnSent(const TcpConnectionPtr &conn) = 0;
        //发送完一个chunk会有一个返回告诉我们发送下一个
        virtual bool OnSentNextChunk(const TcpConnectionPtr &conn) = 0;
        virtual void OnRequest(const TcpConnectionPtr &conn, const HttpRequestPtr &req, const PacketPtr &packet) = 0;
    };
  }
}
