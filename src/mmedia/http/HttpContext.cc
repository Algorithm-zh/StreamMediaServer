#include "HttpContext.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/http/HttpParser.h"
using namespace tmms::mm;
namespace
{
  static std::string CHUNK_EOF = "0\r\n\r\n";
}
HttpContext::HttpContext(EventLoop *loop, const TcpConnectionPtr &conn, HttpHandler *handler)
: loop_{loop}, connection_{conn}, handler_{handler}
{
 
}
 
int32_t HttpContext::Parse(MsgBuffer &buf)  {

  while(buf.ReadableBytes() > 1)
  {
    auto state = http_parser_.Parse(buf);
    if(state == kExpectHttpComplete || state == kExpectChunkComplete)
    {
      if(handler_)
      {
        handler_->OnRequest(connection_, http_parser_.GetHttpRequest(), http_parser_.Chunk());
      }
    }
    else if(state == kExpectError)
    {
      connection_->ForceClose();
    }
  }
  return 1;
}
 
bool HttpContext::PostRequest(const std::string &header_and_body)  {
  if(post_state_ != kHttpContextPostInit)
  {
    return false;
  }
  header_ = header_and_body;
  post_state_ = kHttpContextPostHttp;
  connection_->Send(header_.c_str(), header_.size());

  return true;
}
 
bool HttpContext::PostRequest(const std::string &header, PacketPtr &packet)  {
	
  if(post_state_ != kHttpContextPostInit)
  {
    return false;
  }
  header_ = header;
  out_packet_ = packet;
  post_state_ = kHttpContextPostHttpHeader;
  connection_->Send(header_.c_str(), header_.size());

  return true;
}
 
bool HttpContext::PostRequest(HttpRequestPtr &request)  {
  if(request->IsChunked())
  {
    PostChunkHeader(request->MakeHeaders());
  }
  else if(request->IsStream())
  {
    PostStreamHeader(request->MakeHeaders());
  }
  else
  {
    PostRequest(request->AppendToBuffer());
  }
  return true;
}

bool HttpContext::PostChunkHeader(const std::string &header)  {

  if(post_state_ != kHttpContextPostInit)
  {
    return false;
  }
  header_ = header;
  post_state_ = kHttpContextPostInit;
  connection_->Send(header_.c_str(), header_.size());
  header_sent_ = true;
  return true;
}

void HttpContext::PostChunk(PacketPtr &chunk)  {
  out_packet_ = chunk; 
  if(!header_sent_)//头还没有发送
  {
    post_state_ = kHttpContextPostChunkHeader;
    connection_->Send(header_.c_str(), header_.size());
    header_sent_ = true;
  }
  else
  {
    post_state_ = kHttpContextPostChunkLen;
    char buf[32] = {0,};
    sprintf(buf, "%x\r\n", chunk->PacketSize());
    header_ = std::string(buf);
    connection_->Send(header_.c_str(), header_.size());
  }
}
 
void HttpContext::PostEofChunk()  {
 
  post_state_ = kHttpContextPostChunkEOF;
  connection_->Send(CHUNK_EOF.c_str(), CHUNK_EOF.size());
}
 
bool HttpContext::PostStreamHeader(const std::string &header)  {
  if(post_state_ != kHttpContextPostInit)
  {
    return false;
  }
  header_ = header;
  post_state_ = kHttpContextPostInit;
  connection_->Send(header_.c_str(), header_.size());
  header_sent_ = true;

  return true;
}
 
void HttpContext::PostStreamChunk(PacketPtr &packet)  {
 
  out_packet_ = packet; 
  if(!header_sent_)//头还没有发送
  {
    post_state_ = kHttpContextPostHttpStreamHeader;
    connection_->Send(header_.c_str(), header_.size());
    header_sent_ = true;
  }
  else
  {
    post_state_ = kHttpContextPostHttpStreamChunk;
    connection_->Send(out_packet_->Data(), out_packet_->PacketSize());
  }
}
 
void HttpContext::WriteComplete(const TcpConnectionPtr &conn)  {

  switch (post_state_) {
    case kHttpContextPostInit:
    {
      break;
    }
    case kHttpContextPostHttp:
    {
      post_state_ = kHttpContextPostInit; 
      break;
    }
    case kHttpContextPostHttpHeader:
    {
      post_state_ = kHttpContextPostHttpBody;
      connection_->Send(out_packet_->Data(), out_packet_->PacketSize());
      break;
    }
    case kHttpContextPostHttpBody:
    {
      post_state_ = kHttpContextPostInit;
      break;
    }
    case kHttpContextPostChunkHeader:
    {
      post_state_ = kHttpContextPostChunkLen;
      char buf[32] = {0,};
      sprintf(buf, "%x\r\n", out_packet_->PacketSize());
      header_ = std::string(buf);
      connection_->Send(header_.c_str(), header_.size());
      break;
    }
    case kHttpContextPostChunkLen:
    {
      post_state_ = kHttpContextPostChunkBody;
      connection_->Send(out_packet_->Data(), out_packet_->PacketSize());
      break;
    }
    case kHttpContextPostChunkBody:
    {
      post_state_ = kHttpContextPostInit;
      break;
    }
    case kHttpContextPostChunkEOF:
    {
      post_state_ = kHttpContextPostInit;
      break;
    }
    case kHttpContextPostHttpStreamHeader:
    {
      post_state_ = kHttpContextPostInit;
      break;
    }
    case kHttpContextPostHttpStreamChunk:
    {
      post_state_ = kHttpContextPostInit;
      break;
    }
  }
}
 

