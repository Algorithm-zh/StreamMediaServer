#pragma once
#include "mmedia/base/Packet.h"
#include "mmedia/http/HttpTypes.h"
#include "network/base/MsgBuffer.h"
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
    class HttpParser
    {
    public:
      HttpParser() = default;
      ~HttpParser() = default;
      HttpParserState Parse(MsgBuffer &buf);
      //消息头处理成员函数
      void AddHeader(const std::string &key, const std::string &value);
      void AddHeader(std::string &&key, std::string &&value);
      std::string GetHeader(const std::string &key);
      const std::unordered_map<std::string, std::string> &Headers() const;
      //结果数据成员函数
      const std::string &Method() const;
      const std::string &Version() const;
      uint32_t Code() const;
      const std::string &Path() const;
      const std::string &Query() const;
      const PacketPtr &Chunk() const;
      //其他成员函数
      HttpStatusCode Reason() const;
      void ClearForNextHttp();//一个请求完成后清除状态进入下一个请求的循环
      void ClearForNextChunk();//一个chunk完成后清除状态进入下一个chunk的循环
      bool IsRequest() const;
      
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
      bool is_request_{false};
      HttpStatusCode reason_{kUnknown};//失败的原因

      std::string header_;
      std::unordered_map<std::string, std::string> headers_;
      std::string method_;
      uint32_t code_{0};//响应的code
      std::string version_;
      std::string path_;
      std::string query_;
      PacketPtr chunk_;//保存消息体
    };
  }
}
