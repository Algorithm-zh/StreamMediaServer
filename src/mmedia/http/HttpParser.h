#pragma once
#include "mmedia/base/Packet.h"
#include "mmedia/http/HttpTypes.h"
#include "network/base/MsgBuffer.h"
#include "mmedia/http/HttpRequest.h"
#include <unordered_map>
#include <string>
#include <memory>
namespace tmms
{
  namespace mm
  {
    enum HttpParserState
    {
        kExpectHeaders,
        kExpectNormalBody,
        kExpectStreamBody,
        kExpectHttpComplete,
        kExpectChunkLen,
        kExpectChunkBody,
        kExpectChunkComplete,
        kExpectLastEmptyChunk,
    
        kExpectContinue,
        kExpectError,
    };
    using namespace tmms::network;
    using HttpRequestPtr = std::shared_ptr<HttpRequest>;
    class HttpParser
    {
    public:
      HttpParser() = default;
      ~HttpParser() = default;
      HttpParserState Parse(MsgBuffer &buf);
      //结果数据成员函数
      const PacketPtr &Chunk() const;
      //其他成员函数
      HttpStatusCode Reason() const;
      void ClearForNextHttp();//一个请求完成后清除状态进入下一个请求的循环
      void ClearForNextChunk();//一个chunk完成后清除状态进入下一个chunk的循环
      HttpRequestPtr GetHttpRequest() const
      {
        return req_;
      }
      
    private:
      void ParseStream(MsgBuffer &buf);
      void ParseNormalBody(MsgBuffer &buf);
      void ParseChunk(MsgBuffer &buf);
      void ParseHeaders();
      //解析请求或响应的方法行
      void ProcessMethodLine(const std::string &line);


      HttpParserState state_{kExpectHeaders};
      int32_t current_chunk_length_{0};
      int32_t current_content_length_{0};//NormalBody的长度
      bool is_stream_{false};
      bool is_chunked_{false};
      bool is_request_{true};
      HttpStatusCode reason_{kUnknown};//失败的原因
      std::string header_;
      PacketPtr chunk_;//保存消息体
  
      HttpRequestPtr req_;
    };
  }
}
