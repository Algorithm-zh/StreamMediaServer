#include "HttpParser.h"
#include "mmedia/http/HttpTypes.h"
#include "base/StringUtils.h"
#include "mmedia/base/MMediaLog.h"
#include <algorithm>
#include <cstring>
#include <string>
using namespace tmms::mm;
 
static std::string CRLFCRLF = "\r\n\r\n";
static int32_t kHttpMaxBodySize = 64 * 1024;
namespace
{
  static std::string string_empty;
}
HttpParserState HttpParser::Parse(MsgBuffer &buf)  {
  if(buf.ReadableBytes() == 0)
  {
    return state_;
  }
  switch (state_)
  {
    case kExpectHeaders:
    {
      if(buf.ReadableBytes() > CRLFCRLF.size())
      {
        //查找结束符（这个函数要求前两个参数的类型一致）
        auto *space = std::search(buf.Peek(), (const char*)buf.BeginWrite(), CRLFCRLF);
        if(space != (const char*)buf.BeginWrite())
        {
          //找到结束符,存入header
          auto size = space - buf.Peek();
          header_.assign(buf.Peek(), size);
          buf.Retrieve(size);
          ParseHeaders();//解析header看是否已经结束或错误
          if(state_ == kExpectHttpComplete || state_ == kExpectError)
          {
            return state_;
          }
        }
        else
        {
          if(buf.ReadableBytes() > kHttpMaxBodySize)//超过最大body大小
          {
            reason_ = k400BadRequest;
            state_ = kExpectError;
            return state_;
          }
          //数据不够，等待数据
          return kExpectContinue;;
        }
      }
      else
      {
        return kExpectContinue;
      }
      break;
    }
    case kExpectNormalBody:
    {
      ParseNormalBody(buf);
      break;
    }
    case kExpectStreamBody:
    {
      ParseStream(buf);
      break;
    }
    case kExpectChunkLen:
    {
      auto crlf = buf.FindCRLF();
      if(crlf)
      {
        std::string len(buf.Peek(), crlf); 
        char *end;
        current_chunk_length_ = std::strtol(len.c_str(), &end, 16);//将len按照16进制转换为长整型
        HTTP_DEBUG << " chunk length:" << current_chunk_length_;
        if(current_chunk_length_ < 2 || current_chunk_length_ > 1024 * 1024)
        {
          HTTP_ERROR << "ERROR chunk length:" << current_chunk_length_;
          state_ = kExpectError;
          reason_ = k400BadRequest;
        }
        buf.RetrieveUntil(crlf + 2);
        if(current_chunk_length_ == 0)
        {
          chunk_.reset();//chunk传输结束
          state_ = kExpectChunkComplete;
          return state_;
        }
        else
        {
          state_ = kExpectChunkBody;
        }
      }
      else
      {
        //找不到crlf,数据过大，错误
        if(buf.ReadableBytes() > 32)
        {
          buf.RetrieveAll();
          reason_ = k400BadRequest;
          state_ = kExpectError;
          return state_;
        }
      }
      break;
    }
    case kExpectChunkBody:
    {
      ParseChunk(buf);
      if(state_ == kExpectChunkComplete)
      {
        return state_;
      }
      break;
    }
    default:
    break;
  }
}
 
void HttpParser::ParseStream(MsgBuffer &buf)  {
 
  if(!chunk_) 
  {
    chunk_ = Packet::NewPacket(kHttpMaxBodySize);//固定大小
  }
  auto size = std::min((int)buf.ReadableBytes(), chunk_->Space());
  memcpy(chunk_->Data() + chunk_->PacketSize(), buf.Peek(), size);
  chunk_->UpdatePacketSize(size);
  buf.Retrieve(size);
  if(chunk_->Space() == 0)//一个包填满了
  {
    state_ = kExpectChunkComplete;
  }
}
 
void HttpParser::ParseNormalBody(MsgBuffer &buf)  {
  if(!chunk_) 
  {
    chunk_ = Packet::NewPacket(current_content_length_);
  }
  auto size = std::min((int)buf.ReadableBytes(), chunk_->Space());
  memcpy(chunk_->Data() + chunk_->PacketSize(), buf.Peek(), size);
  chunk_->UpdatePacketSize(size);
  buf.Retrieve(size);
  current_content_length_ -= size;
  if(current_content_length_ == 0)
  {
    state_ = kExpectHttpComplete;
  }
}
 
void HttpParser::ParseChunk(MsgBuffer &buf)  {
 
  if(!chunk_) 
  {
    chunk_ = Packet::NewPacket(current_chunk_length_);
  }
  auto size = std::min((int)buf.ReadableBytes(), chunk_->Space());
  memcpy(chunk_->Data() + chunk_->PacketSize(), buf.Peek(), size);
  chunk_->UpdatePacketSize(size);
  buf.Retrieve(size);
  current_chunk_length_ -= size;
  if(current_chunk_length_ == 0 || chunk_->Space() == 0)
  {
    state_ = kExpectChunkComplete;
  }
}
 
void HttpParser::ParseHeaders()  {
  auto list = base::StringUtils::SplitString(header_, "\r\n"); 
  if(list.size() < 1)
  {
    reason_ = k400BadRequest;
    state_ = kExpectError;
    return;
  }
  //解析请求或响应的方法行
  ProcessMethodLine(list[0]);
  //解析头部
  for(auto &l : list)
  {
    auto pos = l.find_first_of(":");
    if(pos != std::string::npos)
    {
      auto key = l.substr(0, pos);
      auto value = l.substr(pos + 1);

      HTTP_DEBUG << "parse header: " << key << ":" << value;
      AddHeader(std::move(key), std::move(value));
    }
  }
  auto len = GetHeader("content-length");
  if(!len.empty())
  {
    HTTP_TRACE << "content-length:" << len;
    try
    {
      //把长度转换为unsigned long long类型的整数
      current_content_length_ = std::stoull(len);
    }
    catch(...)//捕获所有异常
    {
      //转换失败
      reason_ = k400BadRequest;
      state_ = kExpectError;
      return;
    }
    if(current_content_length_ == 0)
    {
      state_ = kExpectHttpComplete;
    }
    else
    {
      state_ = kExpectNormalBody;
    }
  }
  else
  {
    const std::string &chunk = GetHeader("transfer-encoding");
    if(!chunk.empty() && chunk == "chunked")
    { //判断是否是chunked编码
      is_chunked_ = true;
      state_ = kExpectChunkLen;
    }
    else
    {
      if((!is_request_ && code_ != 200) 
        || (is_request_ && (method_ == "GET" || method_ == "HEAD" || method_ == "OPTION")))
      {
        //不带内容的请求或响应
        current_content_length_ = 0;
        state_ = kExpectHttpComplete;
      }
      else
      {
        //如果都不是就是一个流
        current_content_length_ = -1;//无限大
        is_stream_ = true;
        state_ = kExpectStreamBody;
      }
    }
  }
}
 
void HttpParser::ProcessMethodLine(const std::string &line)  {
 
  HTTP_DEBUG << "parse method line:" << line;
  auto list = base::StringUtils::SplitString(line, " ");
  if(list.size() != 3)
  {
    reason_ = k400BadRequest;
    state_ = kExpectError;
    return;
  }
  std::string str = list[0];
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  if(str[0] == 'h' && str[1] == 't' && str[2] == 't' && str[3] == 'p')
  {
    //响应
    is_request_ = false;
  }
  if(is_request_)
  {
    method_ = std::move(list[0]);
    const std::string &path = list[1];//url 包含路径和参数
    auto pos = path.find_first_of("?");
    if(pos != std::string::npos)
    {
      path_ = path.substr(0, pos);
      query_ = path.substr(pos + 1);
    }
    else
    {
      path_ = path;
    }
    version_ = std::move(list[2]);
    HTTP_DEBUG << "http method:" << method_ 
               << " path:" << path_ 
               << " version:" << version_ 
               << " query:" << query_;
  }
  else
  {
    version_ = list[0];
    code_ = std::atoi(list[1].c_str());
  }

}
 
void HttpParser::AddHeader(const std::string &key, const std::string &value)  {
  std::string k = key;
  //头的属性和大小写无关，所以全部转换成小写
  std::transform(k.begin(), k.end(), k.begin(), ::tolower);
  headers_[k] = value;
}
 
void HttpParser::AddHeader(std::string &&key, std::string &&value)  {
 
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
  headers_[std::move(key)] = std::move(value);
}
 
std::string HttpParser::GetHeader(const std::string &key)  {

  std::string k = key;
  std::transform(k.begin(), k.end(), k.begin(), ::tolower);
  auto iter = headers_.find(k);
  if(iter != headers_.end())
  {
    return iter->second;
  }
  return string_empty;
}
 
const std::unordered_map<std::string,std::string> &HttpParser::Headers() const {
  return headers_; 
}
 
const std::string &HttpParser::Method() const {
  return method_;
}
 
const std::string &HttpParser::Version() const {
  return version_;
}
 
uint32_t HttpParser::Code() const {
  return code_;
}
 
const std::string &HttpParser::Path() const {
  return path_;
}
 
const std::string &HttpParser::Query() const {
  return query_;
}
 
const PacketPtr &HttpParser::Chunk() const {
  return chunk_;
}
 
HttpStatusCode HttpParser::Reason() const {
  return reason_;
}
 
void HttpParser::ClearForNextHttp()  {
 
  state_ = kExpectHeaders;
  header_.clear();
  path_.clear();
  query_.clear();
  current_content_length_ = -1;
  chunk_.reset();
}
 
void HttpParser::ClearForNextChunk()  {
 
  if(is_chunked_)
  {
    state_ = kExpectChunkLen;
    current_chunk_length_ = -1;
  }
  else
  {
    if(is_stream_)
    {
      state_ = kExpectStreamBody;
    }
    else
    {
      state_ = kExpectHeaders;
      current_chunk_length_ = -1;
    }
  }
  chunk_.reset();
}
 
bool HttpParser::IsRequest() const {
	return is_request_;
}
