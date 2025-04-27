#include "HttpRequest.h"
#include "base/StringUtils.h"
#include "HttpUtils.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/http/HttpTypes.h"
#include <algorithm>
#include <sstream>
#include <utility>
using namespace tmms::mm;
 
namespace 
{
  static std::string string_empty;
}
HttpRequest::HttpRequest(bool is_request)
: is_request_(is_request)
{
 
}

const std::unordered_map<std::string,std::string> &HttpRequest::Headers() const {
  return headers_;   
}

void HttpRequest::AddHeader(const std::string &filed, const std::string &value)  {
 
  std::string k = filed;
  //头的属性和大小写无关，所以全部转换成小写
  std::transform(k.begin(), k.end(), k.begin(), ::tolower);
  headers_[k] = value;
}

void HttpRequest::AddHeader(std::string &&key, std::string &&value)  {

  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  headers_[std::move(key)] = std::move(value);
}

void HttpRequest::RemoveHeader(const std::string &key)  {
 
  std::string k = key;
  std::transform(k.begin(), k.end(), k.begin(), ::tolower);
  headers_.erase(k);
}
 
 
const std::string &HttpRequest::GetHeader(const std::string &key) const {

  std::string k = key;
  std::transform(k.begin(), k.end(), k.begin(), ::tolower);
  auto iter = headers_.find(k);
  if(iter != headers_.end())
  {
    return iter->second;
  }
  return string_empty;
}

std::string HttpRequest::MakeHeaders()  {

  std::stringstream ss;
  if(is_request_)
  {
    AppendRequestFirstLine(ss);
  }
  else
  {
    AppendResponseFirstLine(ss);
  }
  for(auto const &h : headers_)
  {
    ss << h.first << ": " << h.second << "\r\n";
  }
  if(!body_.empty())
  {
    ss << "content-length: " << body_.size() << "\r\n";
  }
  else
  {
    ss << "content-length: 0\r\n";
  }
  ss << "\r\n";
  return ss.str();
}
 
void HttpRequest::SetQuery(const std::string &query)  {
  query_ = query; 
}

void HttpRequest::SetQuery(std::string &&query)  {
  query_ = std::move(query);
}

void HttpRequest::setParameter(const std::string &key, const std::string &value)  {
  parameters_[key] = value;
}

void HttpRequest::setParameter(std::string &&key, std::string &&value)  {
  parameters_[std::move(key)] = std::move(value);
}

const std::string &HttpRequest::GetParameter(const std::string &key) const {
  auto iter = parameters_.find(key);
  if(iter != parameters_.end())
  {
    return iter->second;
  }
  return string_empty;
}
 
const std::string &HttpRequest::Query() const {
  return query_;
}
 
void HttpRequest::ParseParameters() {
  auto list = base::StringUtils::SplitString(query_, "&");   
  for(auto const &l : list)
  {
    auto pos = l.find("=");
    if(pos != std::string::npos)
    {
      std::string k = l.substr(0, pos);
      std::string v = l.substr(pos + 1);
      //把左右两端空格去掉
      k = HttpUtils::Trim(k);
      v = HttpUtils::Trim(v);
      setParameter(std::move(k), std::move(v));
    }
  }
}
 
void HttpRequest::SetMethod(const std::string &method)  {
  method_ = HttpUtils::ParseMethod(method); 
}
 
void HttpRequest::SetMethod(HttpMethod method)  {
  method_ = method; 
}
 
HttpMethod HttpRequest::Method() const {
  return method_;
}
 
void HttpRequest::SetVersion(Version v)  {
  version_ = v; 
}
 
void HttpRequest::SetVersion(const std::string &v)  {
  version_ = Version::kUnknown;
  if(v.size() == 8)// http/1.0
  {
    if(v.compare(0, 6, "HTTP/1."))
    {
      if(v[7] == '1')
      {
        version_ = Version::kHttp11;
      }
      else if(v[7] == '0')
      {
        version_ = Version::kHttp10;
      }
    }
  }
}

Version HttpRequest::GetVersion() const {
	return version_;
}
 
void HttpRequest::SetPath(const std::string &path)  {

  if(HttpUtils::NeedUrlDecoding(path))
  {
    path_ = HttpUtils::UrlDecode(path);
  }
  else
  {
    path_ = path;
  }
}
 
const std::string &HttpRequest::Path() const {
  return path_;
}
 
void HttpRequest::SetStatusCode(int32_t code)  {
  code_ = code; 
}

uint32_t HttpRequest::GetStatusCode() const {
  return code_;
}

void HttpRequest::SetBody(const std::string &body)  {
  body_ = body;
}

void HttpRequest::SetBody(std::string &&body)  {
  body_ = std::move(body); 
}

const std::string &HttpRequest::Body() const {
  return body_;
}
 
void HttpRequest::AppendRequestFirstLine(std::stringstream &ss)  {
 
  //请求方法
  switch (method_) {
    case kGet: 
    {
      ss << "GET ";
      break;
    }
    case kPost: 
    {
      ss << "POST ";
      break;
    }
    case kHead: 
    {
      ss << "HEAD ";
      break;
    }
    case kPut: 
    {
      ss << "PUT ";
      break;
    }
    case kDelete: 
    {
      ss << "DELETE ";
      break;
    }
    case kOptions: 
    {
      ss << "OPTIONS ";
      break;
    }
    case kPatch: 
    {
      ss << "PATCH ";
      break;
    }
    case kInvalid: 
    {
      ss << "INVALID ";
      break;
    }
  }
  //路径
  std::stringstream sss; 
  if(!path_.empty())
  {
    sss << path_;
  }
  else
  {
    sss << "/";
  }
  //参数
  if(parameters_.empty())
  {
    sss << "?";
    for(auto iter = parameters_.begin(); iter != parameters_.end(); ++iter)
    {
      if(iter == parameters_.begin())
      {
        sss << iter->first << "=" << iter->second;
      }
      else
      {
        sss << "&" << iter->first << "=" << iter->second;
      }
    }
  }
  ss << HttpUtils::UrlEncode(sss.str()) << " ";
  if(version_ == Version::kHttp10)
  {
    ss << "HTTP/1.0 ";
  }
  else
  {
    ss << "HTTP/1.1 ";
  }
  ss << "\r\n";
}
 
void HttpRequest::AppendResponseFirstLine(std::stringstream &ss)  {

  if(version_ == Version::kHttp10)
  {
    ss << "HTTP/1.0 ";
  }
  else
  {
    ss << "HTTP/1.1 ";
  }
  ss << code_ << " ";
  ss << HttpUtils::ParseStatusMessage(code_);
  ss << "\r\n";
}
 
bool HttpRequest::IsRequest() const {
	return is_request_;
}
 
std::string HttpRequest::AppendToBuffer()  {
  std::stringstream ss;
  ss << MakeHeaders();
  if(!body_.empty())
  {
    ss << body_;
  }
  return ss.str();
}
 
bool HttpRequest::IsStream() const {
	return is_stream_;
}
 
bool HttpRequest::IsChunked() const {
	return is_chunked_;
}
 
void HttpRequest::SetIsStream(bool s)  {
  is_stream_ = s;
}
 
void HttpRequest::SetIsChunked(bool c)  {
  is_chunked_ = c;
}
 

