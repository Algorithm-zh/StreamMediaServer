#pragma once
#include "mmedia/http/HttpHandler.h"
#include "mmedia/http/HttpTypes.h"
#include <sstream>
#include <string>
#include <unordered_map>
#include <memory>
namespace tmms
{
  namespace mm
  {
    class HttpRequest;
    using HttpRequestPtr = std::shared_ptr<HttpRequest>;
    class HttpRequest
    {
    public:
      explicit HttpRequest(bool is_request = true);
      //消息头成员函数
      const std::unordered_map<std::string, std::string> &Headers() const;
      void AddHeader(const std::string &filed, const std::string &value);
      void AddHeader(std::string &&key, std::string &&value);
      void RemoveHeader(const std::string &key);
      const std::string &GetHeader(const std::string &key) const;
      std::string MakeHeaders();//拼装消息头
      //Query成员函数
      void SetQuery(const std::string &query);
      void SetQuery(std::string &&query);
      void setParameter(const std::string &key, const std::string &value);
      void setParameter(std::string &&key, std::string &&value);
      const std::string &GetParameter(const std::string &key) const;
      const std::string &Query() const;
      //Method成员函数
      void SetMethod(const std::string &method);
      void SetMethod(HttpMethod method);
      HttpMethod Method() const;
      //Version和Path成员函数
      void SetVersion(Version v);
      void SetVersion(const std::string &v);
      Version GetVersion() const;
      void SetPath(const std::string &path);
      const std::string &Path() const;
      //code和Body成员函数
      void SetStatusCode(int32_t code);
      uint32_t GetStatusCode() const;
      void SetBody(const std::string &body);
      void SetBody(std::string &&body);
      const std::string &Body() const;
      //内容组装成员函数
      std::string AppendToBuffer();
      bool IsRequest() const;
      bool IsStream() const;
      bool IsChunked() const;
      void SetIsStream(bool s);
      void SetIsChunked(bool c);
      
      static HttpRequestPtr NewHttp400Response();
      static HttpRequestPtr NewHttp404Response();
    private:
      void ParseParameters();
      void AppendRequestFirstLine(std::stringstream &ss);
      void AppendResponseFirstLine(std::stringstream &ss);

      HttpMethod method_{kInvalid};
      Version version_{Version::kUnknown};
      std::string path_;
      std::string query_;
      std::unordered_map<std::string, std::string> headers_;
      std::unordered_map<std::string, std::string> parameters_;
      std::string body_;
      uint32_t code_{0};
      bool is_request_{true};
      bool is_stream_{false};
      bool is_chunked_{false};
    };
  }
}
