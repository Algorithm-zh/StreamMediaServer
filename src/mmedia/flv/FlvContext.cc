#include "FlvContext.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/rtmp/RtmpHeader.h"
#include <cstring>
#include <memory>
#include <sstream>
#include <utility>
using namespace tmms::mm;
//只有音频的FLV头
static char flv_audio_only_header[] =
{
    0x46,/*'F'*/
    0x4c,/*'L'*/
    0x56,/*'V'*/
    0x01,/*version = 1*/
    0x04,/*只有音频*/
    0x00,
    0x00,
    0x00,
    0x09,/*header size*/
};
//只有视频的FLV头
static char flv_video_only_header[] =
{
    0x46,/*'F'*/
    0x4c,/*'L'*/
    0x56,/*'V'*/
    0x01,/*version = 1*/
    0x01,/*只有视频*/
    0x00,
    0x00,
    0x00,
    0x09,/*header size*/
}; 
//都有
static char flv_header[] =
{
    0x46,/*'F'*/
    0x4c,/*'L'*/
    0x56,/*'V'*/
    0x01,/*version = 1*/
    0x05,/*00000 1 0 1 = has audio & video*/
    0x00,
    0x00,
    0x00,
    0x09,/*header size*/
};
FlvContext::FlvContext(const TcpConnectionPtr &conn, MMediaHandler *handler)
:connection_(conn), handler_(handler) 
{
  current_ = out_buffer_;
}
 
void FlvContext::SendFlvHttpHeader(bool has_video, bool has_audio)  {
 
  std::stringstream ss;
  ss << "HTTP/1.1 200 OK\r\n";
  ss << "Access-Control-Allow-Origin: *\r\n";
  ss << "Content-Type: video/x-flv\r\n";
  ss << "Connection: keep-alive\r\n";
  ss << "\r\n";
  http_header_ = ss.str();
  auto header_node = std::make_shared<BufferNode>(http_header_.data(), http_header_.size());
  bufs_.emplace_back(std::move(header_node));
  WriteFlvHeader(has_video, has_audio);
  Send();
}
 
void FlvContext::WriteFlvHeader(bool has_video, bool has_audio)  {
 
  char *header = current_;
  if(!has_audio)
  {
    memcpy(current_, flv_video_only_header, sizeof(flv_video_only_header));
    current_ += sizeof(flv_video_only_header);
  }
  else if(!has_video)
  {
    memcpy(current_, flv_audio_only_header, sizeof(flv_audio_only_header));
    current_ += sizeof(flv_audio_only_header);
  }
  else
  {
    memcpy(current_, flv_header, sizeof(flv_header));
    current_ += sizeof(flv_header);
  }
  auto h = std::make_shared<BufferNode>(header, current_ - header);
  bufs_.emplace_back(std::move(h));
}
 
bool FlvContext::BuildFlvFrame(PacketPtr &pkt, uint32_t timestamp)  {

  out_packets_.emplace_back(pkt);
  char *header = current_;
  char *p = (char *)&previous_size_;
  //Tag header
  //四字节的上一个tag的长度
  *current_++ = p[3];
  *current_++ = p[2];
  *current_++ = p[1];
  *current_++ = p[0];
  //包类型
  *current_++ = GetRtmpPacketType(pkt);
  //数据包长度
  auto mlen = pkt->PacketSize();
  p = (char *)&mlen;
  *current_++ = p[2];
  *current_++ = p[1];
  *current_++ = p[0];
  //时间戳
  p = (char *)&timestamp;
  *current_++ = p[2];
  *current_++ = p[1];
  *current_++ = p[0];
  *current_++ = 0;//timeStampExtended
  
  //chunk stream id
  *current_++ = 0;
  *current_++ = 0;
  *current_++ = 0;

  previous_size_ = mlen + 11;

  auto h = std::make_shared<BufferNode>(header, current_ - header);
  bufs_.emplace_back(std::move(h));
  //tag body
  auto c = std::make_shared<BufferNode>(pkt->Data(), pkt->PacketSize());
  bufs_.emplace_back(std::move(c));
  return true;
}
 
void FlvContext::Send()  {
 
  if(sending_)
  {
    return;
  }
  sending_ = true;
  connection_->Send(bufs_);
}
 
void FlvContext::WriteComplete(const TcpConnectionPtr &conn)  {
  sending_ = false;
  current_ = out_buffer_;
  bufs_.clear();
  out_packets_.clear();

  if(handler_)
  {
    handler_->OnActive(conn);
  }
}
 
bool FlvContext::Ready() const {
	return !sending_;
}
 
char FlvContext::GetRtmpPacketType(PacketPtr &pkt)  {

  if(pkt->IsAudio())
  {
    return kRtmpMsgTypeAudio;
  }
  else if(pkt->IsVideo())
  {
    return kRtmpMsgTypeVideo;
  }
  else if(pkt->IsMeta())
  {
    return kRtmpMsgTypeAMFMeta;
  }
  return 0;
}
