#pragma once

#include "HttpParser.h"
#include "HttpTypes.h"
#include "mmedia/base/Packet.h"
#include "HttpHandler.h"
#include "network/net/EventLoop.h"
#include <string>

namespace tmms
{
    namespace mm
    {
        using namespace tmms::network;
        enum HttpContextPostState
        {
            kHttpContextPostInit,
            kHttpContextPostHttp,
            kHttpContextPostHttpHeader,
            kHttpContextPostHttpBody,
            kHttpContextPostHttpStreamHeader,
            kHttpContextPostHttpStreamChunk,
            kHttpContextPostChunkHeader,
            kHttpContextPostChunkLen,
            kHttpContextPostChunkBody,
            kHttpContextPostChunkEOF
        };
        class HttpContext
        {
        public:
          HttpContext(const TcpConnectionPtr &conn, HttpHandler *handler);
          ~HttpContext() = default;
          //解析成员函数
          int32_t Parse(MsgBuffer &buf);
          //发送成员函数
          bool PostRequest(const std::string &header_and_body);
          bool PostRequest(const std::string &header, PacketPtr &packet);
          bool PostRequest(HttpRequestPtr &request);
          bool PostChunkHeader(const std::string &header);
          void PostChunk(PacketPtr &chunk);
          void PostEofChunk();
          bool PostStreamHeader(const std::string &header);
          void PostStreamChunk(PacketPtr &packet);
          //其他成员函数
          //推动状态机流转的
          void WriteComplete(const TcpConnectionPtr &conn);
        private:
          TcpConnectionPtr connection_;
          HttpParser http_parser_;
          std::string header_;
          PacketPtr out_packet_;
          HttpContextPostState post_state_{kHttpContextPostInit};
          bool header_sent_{false};
          HttpHandler *handler_{nullptr};
          
        };
    }
}
