#include "RtmpContext.h"
#include "../base/MMediaLog.h"
#include "RtmpHandShake.h"
#include "RtmpHeader.h"
#include "../base/BytesReader.h"
#include "../base/BytesWriter.h"
#include "amf/AMFAny.h"
#include "amf/AMFObject.h"
#include "../base/Packet.h"
#include "../../base/StringUtils.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>
using namespace tmms::mm;
 
RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *handler, bool client)
:handshake_(conn, client), connection_(conn), rtmp_handler_(handler), is_client_(client)  
{
  commands_["connect"] = std::bind(&RtmpContext::HandleConnect, this, std::placeholders::_1); 
  commands_["createStream"] = std::bind(&RtmpContext::HandleCreateStream, this, std::placeholders::_1);
  commands_["_result"] = std::bind(&RtmpContext::HandleResult, this, std::placeholders::_1);
  commands_["_error"] = std::bind(&RtmpContext::HandleError, this, std::placeholders::_1);
  commands_["play"] = std::bind(&RtmpContext::HandlePlay, this, std::placeholders::_1);
  commands_["publish"] = std::bind(&RtmpContext::HandlePublish, this, std::placeholders::_1);
  out_current_ = out_buffer_;
}
 
int32_t RtmpContext::Parse(MsgBuffer &buf)  {
  int32_t ret = 0;
  if(state_ == kRtmpHandShake)
  {
    ret = handshake_.HandShake(buf);
    //握手成功
    if(ret == 0)
    {
      state_ = kRtmpMessage;
      if(is_client_)
      {
        SendConnect();
      }
      if(buf.ReadableBytes() > 0)
      {
        return Parse(buf);
      }
    }
    else if(ret == -1)
    {
      RTMP_ERROR << "rtmp handshake failed";
    }
    else if(ret == 2)
    {
      state_ = kRtmpWaitingDone;
    }
  }
  else if(state_ == kRtmpMessage)
  {
    auto ret = ParseMessage(buf);
    last_left_ = buf.ReadableBytes();
  }
  return ret;
}
 
void RtmpContext::OnWriteComplete()  {
 
  if(state_ == kRtmpHandShake)
  {
    handshake_.WriteComplete();
  }
  else if(state_ == kRtmpWaitingDone)
  {
    state_ = kRtmpMessage;
    if(is_client_)
    {
      SendConnect();
    }
  }
  else if(state_ == kRtmpMessage)
  {
    //发送完毕后，再检测是否还有数据要发送
    CheckAndSend();
  }
}
 
void RtmpContext::StartHandShake()  {
 
  handshake_.Start();
}
 
 
int32_t RtmpContext::ParseMessage(MsgBuffer &buf)  {
  uint8_t fmt;
  uint32_t csid, msg_len = 0, msg_sid = 0, timestamp = 0;
  uint8_t msg_type = 0;
  uint32_t total_bytes = buf.ReadableBytes();
  //每读取一个字节就++
  int32_t parsed = 0;

  in_bytes_ += buf.ReadableBytes() - last_left_;
  //接收到消息之后判断是否要recv一个确认消息
  SendBytesRecv();

  while(total_bytes > 1)
  {
    const char *pos = buf.Peek();
    parsed = 0; 
    //Basic Header
    fmt = (*pos >> 6) & 0x03;
    csid = *pos & 0x0F;
    parsed ++;
    if(csid == 0)
    {
      if(total_bytes < 2)
      {
        //数据不过
        return 1;
      }
      csid = 64;
      csid += *((uint8_t*)(pos + parsed));
      parsed ++;
    }
    else if(csid == 1)
    {
      if(total_bytes < 3)
      {
        return 1;
      }     
      csid = 64;
      //按字节方式获取该位置的 1 字节值
      csid += *((uint8_t*)(pos + parsed));
      //读取高8位
      parsed ++;
      //equal to (*((uint8_t*)(pos + parsed))) << 8;
      csid += 256* (*((uint8_t*)(pos + parsed)));
      parsed ++;
    }

    int size = total_bytes - parsed;
    //lack of data
    if(size == 0 || (fmt == 0 && size < 11) || (fmt == 1 && size < 7) || (fmt == 2 && size < 3))
    {
      return 1;
    }
    //parse the message header
    msg_len = 0;
    msg_sid = 0;
    timestamp = 0;
    msg_type = 0;
    int32_t ts = 0;
    RtmpMsgHeaderPtr &prev = in_message_headers_[csid];
    if(!prev)
    {
      prev = std::make_shared<RtmpMsgHeader>();
    }
    if(fmt == kRtmpFmt0)
    {
      ts = BytesReader::ReadUint24T(pos + parsed);
      parsed += 3;
      //fmt0 haven't delta
      in_deltas_[csid] = 0;//下个message检测这个值没有时间差
      msg_len = BytesReader::ReadUint24T(pos + parsed);
      parsed += 3;
      msg_type = BytesReader::ReadUint8T(pos + parsed);
      parsed ++;
      //msg_sid is 4 bytes and little endian. it can directly copy to uint32_t
      memcpy(&msg_sid, pos + parsed, 4);
      parsed += 4;
    }
    else if(fmt == kRtmpFmt1)
    {
      ts = BytesReader::ReadUint24T(pos + parsed);
      parsed += 3;
      in_deltas_[csid] = ts;
      timestamp = ts + prev->timestamp;
      msg_len = BytesReader::ReadUint24T(pos + parsed);
      parsed += 3;
      msg_type = BytesReader::ReadUint8T(pos + parsed);
      parsed ++;
      msg_sid = prev->msg_sid;
    }
    else if(fmt == kRtmpFmt2)
    {
      ts = BytesReader::ReadUint24T(pos + parsed);
      parsed += 3;
      in_deltas_[csid] = ts;
      timestamp = ts + prev->timestamp;
      msg_len = prev->msg_len;
      msg_type = prev->msg_type;
      msg_sid = prev->msg_sid;
    }
    else if(fmt == kRtmpFmt3)
    {
      timestamp = in_deltas_[csid] + prev->timestamp;
      msg_len = prev->msg_len;
      msg_type = prev->msg_type;
      msg_sid = prev->msg_sid;
    }

    //calculate extended timestamp
    //check ext if open
    bool ext = (ts == 0xFFFFFF);
    //fmt3需要看上一个ex是否开启
    if(fmt == kRtmpFmt3)
    {
      ext = in_ext_[csid];
    }
    in_ext_[csid] = ext;
    //already open ext
    if(ext)
    {
      if(total_bytes - parsed < 4)
      {
        return 1;
      }
      ts = BytesReader::ReadUint32T(pos + parsed);
      parsed += 4;
      if(fmt != kRtmpFmt0)
      {
        timestamp = ts + prev->timestamp;
        in_deltas_[csid] = ts;
      }
    }
    
    PacketPtr &packet = in_packets_[csid];
    if(!packet)
    {
      packet = Packet::NewPacket(msg_len);
    }
    //将msgHeader存储到Packet的ext里
    RtmpMsgHeaderPtr header = packet->Ext<RtmpMsgHeader>();
    if(!header)
    {
      header = std::make_shared<RtmpMsgHeader>();
      packet->SetExt(header);
    }
    header->cs_id = csid;
    header->timestamp = timestamp;
    header->msg_len = msg_len;
    header->msg_type = msg_type;
    header->msg_sid = msg_sid;

    int bytes = std::min(packet->Space(), in_chunk_size_);
    if(total_bytes - parsed < bytes)
    {
      return 1;
    }
    //body = Chunk Data = message data part
    //开始的PacketSize为空（因为还没有设置）
    char *body = packet->Data() + packet->PacketSize();
    memcpy(body, pos + parsed, bytes); 
    packet->UpdatePacketSize(bytes); 
    parsed += bytes;

    buf.Retrieve(parsed);
    total_bytes -= parsed;

    prev->cs_id = csid;
    prev->msg_len = msg_len;
    prev->msg_sid = msg_sid;
    prev->timestamp = timestamp;
    prev->msg_type = msg_type;

    //no space. parse finished
    if(packet->Space() == 0)
    {
      packet->SetPacketType(msg_type);
      packet->SetTimeStamp(timestamp);
      MessageComplete(std::move(packet));
      packet.reset();
    }
  }
  return 1;
}
 
void RtmpContext::MessageComplete(PacketPtr &&data)  {
 
  RTMP_TRACE << "recv message type:" << data->PacketType() << ", len = " << data->PacketSize();
  auto type = data->PacketType();
  switch (type) {
    case kRtmpMsgTypeChunkSize:
    {
      HandleChunkSize(data);
      break;
    }
    case kRtmpMsgTypeBytesRead:
    {
      RTMP_TRACE << "message bytes read recv";
      break;
    }
    case kRtmpMsgTypeUserControl:
    {
      HandleUserMessage(data);
      break;
    }
    case kRtmpMsgTypeWindowACKSize:
    {
      HandleAckWindowSize(data);
      break;
    }
    case kRtmpMsgTypeAMF3Message:
    {
      HandleAmfCommand(data, true);
      break;
    }
    case kRtmpMsgTypeAMFMessage:
    {
      HandleAmfCommand(data);
      break;
    }
    case kRtmpMsgTypeAMFMeta:
    case kRtmpMsgTypeAMF3Meta:
    case kRtmpMsgTypeAudio:
    case kRtmpMsgTypeVideo:
    {
      SetPacketType(data);
      if(rtmp_handler_)
      {
        rtmp_handler_->OnRecv(connection_, data);  
      }
      break;
    }
    default:
    RTMP_ERROR << "not support message type:" << type;
    break;
  }
}

 
bool RtmpContext::BuildChunk(const PacketPtr &packet, uint32_t timestamp, bool fmt0)  {

  RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();
  if(h)
  {
    RtmpMsgHeaderPtr &prev = out_message_headers_[h->cs_id];
    //使用delta就代表不使用fmt0
    //使用fmt0的条件：强制使用fmt0, 第一个包, 时间戳小于上一个包, 包号相同
    bool use_delta = !fmt0 && !prev && timestamp >= prev->timestamp && h->msg_sid == prev->msg_sid;
    if(!prev)
    {
      prev = std::make_shared<RtmpMsgHeader>();
    }
    int fmt = kRtmpFmt0;
    if(use_delta)
    {
      fmt = kRtmpFmt1;
      timestamp -= prev->timestamp;
      if(h->msg_type == prev->msg_type && h->msg_len == prev->msg_len)
      {
        fmt = kRtmpFmt2;
        if(timestamp == out_deltas_[h->cs_id])
        {
          fmt = kRtmpFmt3;
        }
      }
    }

    char *p = out_current_;
    //csid < 64 csid写到Basic Header第一个字节的后6位
    if(h->cs_id < 64)
    {
      *p ++ = (char)((fmt << 6) | h->cs_id);
    }
    else if(h->cs_id < (64 + 256))
    {
      *p ++ = (char)((fmt << 6) | 0);
      *p ++ = (char)(h->cs_id - 64);
    }
    else
    {
      *p ++ = (char)((fmt << 6) | 1);
      uint16_t cs = h->cs_id - 64;
      memcpy(p, &cs, sizeof(uint16_t));
      p += sizeof(uint16_t);//移动指针到新的位置
    }

    auto ts = timestamp;
    if(timestamp >= 0xFFFFFF)
    {
      //启用extended timestamp
      ts = 0xFFFFFF;
    }
  
    if(fmt == kRtmpFmt0)
    {
      p += BytesWriter::WriteUint24T(p, ts);
      p += BytesWriter::WriteUint8T(p, h->msg_len);
      p += BytesWriter::WriteUint8T(p, h->msg_type);

      memcpy(p, &h->msg_sid, 4);
      p += 4;
      out_deltas_[h->cs_id] = 0;
    }
    else if(fmt == kRtmpFmt1)
    {
      p += BytesWriter::WriteUint24T(p, ts);
      p += BytesWriter::WriteUint8T(p, h->msg_len);
      p += BytesWriter::WriteUint8T(p, h->msg_type);
      out_deltas_[h->cs_id] = timestamp;
    }
    else if(fmt == kRtmpFmt2)
    {
      p += BytesWriter::WriteUint24T(p, ts);
      out_deltas_[h->cs_id] = timestamp;
    }

    if(ts == 0xFFFFFF)
    {
      memcpy(p, &timestamp, 4);
      p += 4;
    }
    BufferNodePtr node = std::make_shared<BufferNode>(out_current_, p - out_current_);
    sending_bufs_.emplace_back(std::move(node));
    out_current_ = p;

    prev->cs_id = h->cs_id;
    prev->msg_len = h->msg_len;
    prev->msg_sid = h->msg_sid;
    prev->msg_type = h->msg_type;
    if(fmt == kRtmpFmt0)
    {
      prev->timestamp = timestamp;
    }
    else
    {
      //非fmt0是时间差,前面已经减去了prev的timestamp
      prev->timestamp += timestamp; 
    }

    const char *body = packet->Data();
    int32_t bytes_parsed = 0;
    while(true)
    {
      const char *chunk = body + bytes_parsed;
      int32_t size = h->msg_len - bytes_parsed;
      size = std::min(size, packet->Space());

      BufferNodePtr node = std::make_shared<BufferNode>((void*)chunk, size);
      sending_bufs_.emplace_back(std::move(node));
      bytes_parsed += size;
      if(bytes_parsed < h->msg_len)
      {
        //没发完，剩下的都是封装成fmt3发送
        //创建一个header继续发
        if(out_current_ - out_buffer_ >= 4096)
        {
          RTMP_ERROR << "rtmp had no enough out header buffer.";
          break;
        }
        
        char *p = out_current_;
        if(h->cs_id < 64)
        {
          *p ++ = (char)(0xc0 | h->cs_id);
        }
        else if(h->cs_id < (64 + 256))
        {
          *p ++ = (char)(0xc0 | 0);
          *p ++ = (char)(h->cs_id - 64);
        }
        else
        {
          *p ++ = (char)(0xc0 | 1);
          uint16_t cs = h->cs_id - 64;
          memcpy(p, &cs, sizeof(uint16_t));
          p += sizeof(uint16_t);//移动指针到新的位置
        }

        if(ts == 0xFFFFFF)
        {
          memcpy(p, &timestamp, 4);
          p += 4;
        }

        BufferNodePtr node = std::make_shared<BufferNode>(out_current_, p - out_current_);
        sending_bufs_.emplace_back(std::move(node));
        out_current_ = p;
      }
      else
      {
        break;
      }
    }
    out_sending_packets_.emplace_back(packet);
    return true;
  }
  return false;
}

//核心函数:构建要发送的chunk包
bool RtmpContext::BuildChunk(PacketPtr &&packet, uint32_t timestamp, bool fmt0)  {

  RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();
  if(h)
  {
    RtmpMsgHeaderPtr &prev = out_message_headers_[h->cs_id];
    //使用delta就代表不使用fmt0
    //使用fmt0的条件：强制使用fmt0, 第一个包, 时间戳小于上一个包, 包号相同
    bool use_delta = !fmt0 && prev && timestamp >= prev->timestamp && h->msg_sid == prev->msg_sid;
    if(!prev)
    {
      prev = std::make_shared<RtmpMsgHeader>();
    }
    int fmt = kRtmpFmt0;
    if(use_delta)
    {
      fmt = kRtmpFmt1;
      timestamp -= prev->timestamp;
      if(h->msg_type == prev->msg_type && h->msg_len == prev->msg_len)
      {
        fmt = kRtmpFmt2;
        if(timestamp == out_deltas_[h->cs_id])
        {
          fmt = kRtmpFmt3;
        }
      }
    }

    char *p = out_current_;
    //csid < 64 csid写到Basic Header第一个字节的后6位
    if(h->cs_id < 64)
    {
      *p ++ = (char)((fmt << 6) | h->cs_id);
    }
    else if(h->cs_id < (64 + 256))
    {
      *p ++ = (char)((fmt << 6) | 0);
      *p ++ = (char)(h->cs_id - 64);
    }
    else
    {
      *p ++ = (char)((fmt << 6) | 1);
      uint16_t cs = h->cs_id - 64;
      memcpy(p, &cs, sizeof(uint16_t));
      p += sizeof(uint16_t);//移动指针到新的位置
    }

    auto ts = timestamp;
    if(timestamp >= 0xFFFFFF)
    {
      //启用extended timestamp
      ts = 0xFFFFFF;
    }
  
    if(fmt == kRtmpFmt0)
    {
      p += BytesWriter::WriteUint24T(p, ts);
      //这tm把24写成8了 让我找了整整两天!!!!!!!!!!!
      p += BytesWriter::WriteUint24T(p, h->msg_len);
      p += BytesWriter::WriteUint8T(p, h->msg_type);

      memcpy(p, &h->msg_sid, 4);
      p += 4;
      out_deltas_[h->cs_id] = 0;
    }
    else if(fmt == kRtmpFmt1)
    {
      p += BytesWriter::WriteUint24T(p, ts);
      p += BytesWriter::WriteUint24T(p, h->msg_len);
      p += BytesWriter::WriteUint8T(p, h->msg_type);
      out_deltas_[h->cs_id] = timestamp;
    }
    else if(fmt == kRtmpFmt2)
    {
      p += BytesWriter::WriteUint24T(p, ts);
      out_deltas_[h->cs_id] = timestamp;
    }

    if(ts == 0xFFFFFF)
    {
      memcpy(p, &timestamp, 4);
      p += 4;
    }
    BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
    sending_bufs_.emplace_back(std::move(nheader));
    out_current_ = p;

    prev->cs_id = h->cs_id;
    prev->msg_len = h->msg_len;
    prev->msg_sid = h->msg_sid;
    prev->msg_type = h->msg_type;
    if(fmt == kRtmpFmt0)
    {
      prev->timestamp = timestamp;
    }
    else
    {
      //非fmt0是时间差,前面已经减去了prev的timestamp
      prev->timestamp += timestamp; 
    }

    const char *body = packet->Data();
    int32_t bytes_parsed = 0;
    while(true)
    {
      const char *chunk = body + bytes_parsed;
      int32_t size = h->msg_len - bytes_parsed;
      size = std::min(size, out_chunk_size_);

      BufferNodePtr node = std::make_shared<BufferNode>((void *)chunk, size);
      sending_bufs_.emplace_back(std::move(node));
      bytes_parsed += size;
      if(bytes_parsed < h->msg_len)
      {
        //没发完，剩下的都是封装成fmt3发送
        //创建一个header继续发
        if(out_current_ - out_buffer_ >= 4096)
        {
          
          RTMP_ERROR << "rtmp had no enough out header buffer.";
          break;
        }
        
        char *p = out_current_;
        if(h->cs_id < 64)
        {
          *p ++ = (char)(0xc0 | h->cs_id);
        }
        else if(h->cs_id < (64 + 256))
        {
          *p ++ = (char)(0xc0 | 0);
          *p ++ = (char)(h->cs_id - 64);
        }
        else
        {
          *p ++ = (char)(0xc0 | 1);
          uint16_t cs = h->cs_id - 64;
          memcpy(p, &cs, sizeof(uint16_t));
          p += sizeof(uint16_t);//移动指针到新的位置
        }

        if(ts == 0xFFFFFF)
        {
          memcpy(p, &timestamp, 4);
          p += 4;
        }

        BufferNodePtr nheader = std::make_shared<BufferNode>(out_current_, p - out_current_);
        sending_bufs_.emplace_back(std::move(nheader));
        out_current_ = p;
      }
      else
      {
        break;
      }
    }
    out_sending_packets_.emplace_back(std::move(packet));
    return true;
  }
  return false;
}
 

void RtmpContext::Send()  {
 
  if(sending_) return;
  sending_ = true;
  //最多发送10个包
  for(int i = 0; i < 10; i ++)
  {
    if(out_waiting_queue_.empty())
    {
      break;
    }
    PacketPtr packet = std::move(out_waiting_queue_.front());
    out_waiting_queue_.pop_front();
    BuildChunk(std::move(packet));
  }
  connection_->Send(sending_bufs_);
}
 
void RtmpContext::CheckAndSend()  {
  
  sending_ = false;
  out_current_ = out_buffer_;
  sending_bufs_.clear();
  out_sending_packets_.clear();

  if(!out_waiting_queue_.empty())
  {
    //不为空，继续发送
    Send();
  }
  else
  {
    if(rtmp_handler_)
    {
      //发送完毕，激活上层，让上层决定处理
      rtmp_handler_->OnActive(connection_);
    }
  }
}
 
bool RtmpContext::Ready() const {
	return !sending_;
}
 
void RtmpContext::PushOutQueue(PacketPtr &&packet)  {
 
  out_waiting_queue_.emplace_back(std::move(packet));
  Send();
}
 
 
void RtmpContext::SendSetChunkSize()  {
  PacketPtr packet = Packet::NewPacket(64); //applied for more memory
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  if(header)
  {
    //csid fixed is 2 and msg_sid fixed is 0
    header->cs_id = kRtmpCSIDCommand; 
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeChunkSize;
    header->timestamp = 0;
    header->msg_sid = kRtmpMsID0;
    packet->SetExt(header);
  }

  char *body = packet->Data();

  header->msg_len = BytesWriter::WriteUint32T(body, out_chunk_size_);
  packet->SetPacketSize(header->msg_len);
  RTMP_DEBUG << "send chunk size " << out_chunk_size_ << "to host:" << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::SendAckWindowSize()  {
 
  PacketPtr packet = Packet::NewPacket(64); //applied for more memory
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  
  if(header)
  {
    //csid fixed is 2 and msg_sid fixed is 0
    header->cs_id = kRtmpCSIDCommand; 
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeWindowACKSize;
    header->timestamp = 0;
    header->msg_sid = kRtmpMsID0;
    packet->SetExt(header);
  }

  char *body = packet->Data();
  header->msg_len = BytesWriter::WriteUint32T(body, ack_size_);
  packet->SetPacketSize(header->msg_len);
  RTMP_DEBUG << "send ack window size " << ack_size_ << "to host: " << connection_->PeerAddr().ToIpPort();
  
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::SendSetPeerBandwidth()  {
 
  PacketPtr packet = Packet::NewPacket(64); //applied for more memory
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  if(header)
  {
    //csid fixed is 2 and msg_sid fixed is 0
    header->cs_id = kRtmpCSIDCommand; 
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeSetPeerBW;
    header->timestamp = 0;
    header->msg_sid = kRtmpMsID0;
    packet->SetExt(header);
  }

  char *body = packet->Data();

  body += BytesWriter::WriteUint32T(body, ack_size_);
  *body ++ = 0x02;//dynamic limit
  header->msg_len = 5;
  packet->SetPacketSize(5);
  RTMP_DEBUG << "send band width " << out_chunk_size_ << "to host: " << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::SendBytesRecv()  {
 
  if(in_bytes_ >= ack_size_)
  {
    PacketPtr packet = Packet::NewPacket(64); //applied for more memory
    RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();

    if(header)
    {
      //csid fixed is 2 and msg_sid fixed is 0
      header->cs_id = kRtmpCSIDCommand; 
      header->msg_len = 0;
      header->msg_type = kRtmpMsgTypeBytesRead;
      header->timestamp = 0;
      header->msg_sid = kRtmpMsID0;
      packet->SetExt(header);
    }

    char *body = packet->Data();

    header->msg_len = BytesWriter::WriteUint32T(body, in_bytes_);
    packet->SetPacketSize(header->msg_len);
    PushOutQueue(std::move(packet));
    in_bytes_ = 0;
  }
}
 
//注意用户控制消息只处理了两个 
void RtmpContext::SendUserCtrlMessage(short nType, uint32_t value1, uint32_t value2)  {
 
  PacketPtr packet = Packet::NewPacket(64); //applied for more memory
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  if(header)
  {
    //csid fixed is 2 and msg_sid fixed is 0
    header->cs_id = kRtmpCSIDCommand; 
    header->msg_len = 0;
    header->msg_type = kRtmpMsgTypeUserControl;
    header->timestamp = 0;
    header->msg_sid = kRtmpMsID0;
    packet->SetExt(header);
  }

  char *body = packet->Data();
  char *p = body;

  p += BytesWriter::WriteUint16T(body, nType);
  p += BytesWriter::WriteUint32T(p, value1);
  if(nType == kRtmpEventTypeSetBufferLength)
  {
    //只有设置 buffer 长度是两个变量
    p += BytesWriter::WriteUint32T(p, value2);
  }
  packet->SetPacketSize(header->msg_len);
  RTMP_DEBUG << "set user control type: " << nType << ", value1: " << value1 << ", value2: " << value2;
  PushOutQueue(std::move(packet));

}
 
void RtmpContext::HandleChunkSize(PacketPtr &packet)  {
 
  if(packet->PacketSize() >= 4)
  {
    auto size = BytesReader::ReadUint32T(packet->Data());
    RTMP_DEBUG << "recv chunk size in_chunk_size: " << in_chunk_size_ 
      << "change to " << size << "host : " << connection_->PeerAddr().ToIpPort();
    in_chunk_size_ = size;
  }
  else
  {
    RTMP_ERROR << "invalid chunk size packet msg_len:" << packet->PacketSize()
    << " host:" << connection_->PeerAddr().ToIpPort();
  }
}
 
void RtmpContext::HandleAckWindowSize(PacketPtr &packet)  {

  if(packet->PacketSize() >= 4)
  {
    auto size = BytesReader::ReadUint32T(packet->Data());
    RTMP_DEBUG << "recv ack window size in_chunk_size: " << ack_size_ 
      << "change to " << size << "host : " << connection_->PeerAddr().ToIpPort();
    ack_size_ = size;
  }
  else
  {
    RTMP_ERROR << "invalid ack windows size packet msg_len:" << packet->PacketSize() 
      << "host : " << connection_->PeerAddr().ToIpPort();
  }
}
 
void RtmpContext::HandleUserMessage(PacketPtr &packet)  {
 
  auto msg_len = packet->PacketSize();
  if(msg_len < 6)
  {
    RTMP_ERROR << "invalid user control packet msg_len:" 
      << packet->PacketSize() << "host : " << connection_->PeerAddr().ToIpPort();
    return;
  }
  char *body = packet->Data();
  auto type = BytesReader::ReadUint16T(body);
  auto value = BytesReader::ReadUint32T(body + 2);
  RTMP_TRACE << "recv user control type:" << type << " value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
  switch(type)
  {
    case kRtmpEventTypeStreamBegin:
    {
      RTMP_TRACE << "recv stream begin value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
      break;
    }
    case kRtmpEventTypeStreamEOF:
    {
      RTMP_TRACE << "recv stream eof value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
      break;
    }
    case kRtmpEventTypeStreamDry:
    {
      RTMP_TRACE << "recv stream dry value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
      break;
    }
    case kRtmpEventTypeSetBufferLength:
    {
      RTMP_TRACE << "recv set buffer length value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
      if(msg_len < 10)
      {
        RTMP_ERROR << "invalid user control packet msg_len:" 
        << packet->PacketSize() << "host : " << connection_->PeerAddr().ToIpPort();
        return;
      }
      break;
    }
    case kRtmpEventTypeStreamsRecorded:
    {
      RTMP_TRACE << "recv stream recorded value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
      break;
    }
    case kRtmpEventTypePingRequest:
    {
      RTMP_TRACE << "recv ping request value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
      SendUserCtrlMessage(kRtmpEventTypePingResponse, value, 0);
      break;
    }
    case kRtmpEventTypePingResponse:
    {
      RTMP_TRACE << "recv ping response value:"<< value << " host:" << connection_->PeerAddr().ToIpPort();
      break;
    }
    default:
      break;
  }
}

 
void RtmpContext::HandleAmfCommand(PacketPtr &data, bool amf3)  {
  
  RTMP_TRACE << "amf message len:" << data->PacketSize() << "host:" << connection_->PeerAddr().ToIpPort();

  const char *body = data->Data();
  int32_t msg_len = data->PacketSize();
  
  if(amf3)
  {
    //不处理第一个字节
    body += 1;
    msg_len -= 1;
  }

  AMFObject obj;
  if(obj.Decode(body, msg_len) < 0)
  {
    RTMP_ERROR << "decode amf message failed, host:" << connection_->PeerAddr().ToIpPort();
  }
  obj.Dump();

  const std::string &method = obj.Property(0)->String();
  RTMP_TRACE << "amf command:" << method << " host:" << connection_->PeerAddr().ToIpPort();
  auto iter = commands_.find(method);
  if(iter == commands_.end())
  {
    RTMP_TRACE << "not support method" << method << " host:" << connection_->PeerAddr().ToIpPort();
    return ;
  }
  iter->second(obj);
}
 
//客户端发送
void RtmpContext::SendConnect()  {
 
  SendSetChunkSize();
  PacketPtr packet = Packet::NewPacket(1024);
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  header->cs_id = kRtmpCSIDAMFIni;
  header->msg_sid = 0;
  header->msg_len = 0;
  header->msg_type = kRtmpMsgTypeAMFMessage;
  packet->SetExt(header);

  char *body = packet->Data();
  char *p = body;

  p += AMFAny::EncodeString(p, "connect");
  p += AMFAny::EncodeNumber(p, 1.0);//设置id
  *p ++ = kAMFObject;
  p += AMFAny::EncodeNamedString(p, "app", app_);//推流点
  p += AMFAny::EncodeNamedString(p, "tcUrl", tc_url_);//推流url
  p += AMFAny::EncodeNamedBoolean(p, "fpad", false);//是否用代理
  p += AMFAny::EncodeNamedNumber(p, "capabilities", 31.0);
  p += AMFAny::EncodeNamedNumber(p, "audioCodecs", 1639.0);
  p += AMFAny::EncodeNamedNumber(p, "videoCodecs", 252.0);
  p += AMFAny::EncodeNamedNumber(p, "videoFunction", 1.0);//支持seek
  //结束符号
  *p ++ = 0x00;
  *p ++ = 0x00;
  *p ++ = 0x09;

  header->msg_len = p - body;
  packet->SetPacketSize(header->msg_len);
  RTMP_TRACE << "send connect message len:" << header->msg_len << "host:" << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::HandleConnect(AMFObject &obj)  {
 
  bool amf3 = false;
  tc_url_ = obj.Property("tcUrl")->String();
  AMFObjectPtr sub_obj = obj.Property(2)->Object();
  if(sub_obj)
  {
    app_ = sub_obj->Property("app")->String();
    if(sub_obj->Property("objectEncoding"))
    {
      amf3 = sub_obj->Property("objectEncoding")->Number() == 3.0;
    }
  }

  RTMP_TRACE << " recv connect tcUrl:" << tc_url_ << " app:" << app_ << " amf3:" << amf3;
  SendAckWindowSize();
  SendSetPeerBandwidth();
  SendSetChunkSize();
  //回复
  PacketPtr packet = Packet::NewPacket(1024);
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  header->cs_id = kRtmpCSIDAMFIni;
  header->msg_sid = 0;
  header->msg_len = 0;
  header->msg_type = kRtmpMsgTypeAMFMessage;
  packet->SetExt(header);

  char *body = packet->Data();
  char *p = body;

  p += AMFAny::EncodeString(p, "_result");
  p += AMFAny::EncodeNumber(p, 1.0);
  *p ++ = kAMFObject;
  p += AMFAny::EncodeNamedString(p, "fmsVer", "FMS/3,0,1,123");
  p += AMFAny::EncodeNamedNumber(p, "capabilities", 31);
  *p ++ = 0x00;
  *p ++ = 0x00;
  *p ++ = 0x09;
  *p ++ = kAMFObject;
  p += AMFAny::EncodeNamedString(p, "level", "status");
  p += AMFAny::EncodeNamedString(p, "code", "NetConnection.Connect.Success");
  p += AMFAny::EncodeNamedString(p, "description", "Connection succeed.");
  p += AMFAny::EncodeNamedNumber(p, "objectEncoding", amf3 ? 3.0 : 0);
  *p ++ = 0x00;
  *p ++ = 0x00;
  *p ++ = 0x09;

  header->msg_len = p - body;
  packet->SetPacketSize(header->msg_len);
  RTMP_TRACE << "connect result len:" << header->msg_len << "host:" << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));

}
 
void RtmpContext::SendCreateStream()  {
 
 
  //SendSetChunkSize();
  PacketPtr packet = Packet::NewPacket(1024);
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  header->cs_id = kRtmpCSIDAMFIni;
  header->msg_sid = 0;
  header->msg_len = 0;
  header->msg_type = kRtmpMsgTypeAMFMessage;
  packet->SetExt(header);

  char *body = packet->Data();
  char *p = body;

  p += AMFAny::EncodeString(p, "createStream");
  p += AMFAny::EncodeNumber(p, 4.0);
  *p ++ = kAMFNull;

  header->msg_len = p - body;
  packet->SetPacketSize(header->msg_len);
  RTMP_TRACE << "send createStream message len:" << header->msg_len << "host:" << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::HandleCreateStream(AMFObject &obj)  {
  
  auto tran_id = obj.Property(1)->Number();
  PacketPtr packet = Packet::NewPacket(1024);
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  header->cs_id = kRtmpCSIDAMFIni;
  header->msg_sid = 0;
  header->msg_len = 0;
  header->msg_type = kRtmpMsgTypeAMFMessage;
  packet->SetExt(header);

  char *body = packet->Data();
  char *p = body;
  p += AMFAny::EncodeString(p, "_result");
  p += AMFAny::EncodeNumber(p, tran_id);
  *p ++ = kAMFNull;
  p += AMFAny::EncodeNumber(p, kRtmpMsID1);

  header->msg_len = p - body;
  packet->SetPacketSize(header->msg_len);
  RTMP_TRACE << "createStream result len:" << header->msg_len << "host:" << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::SendStatus(const std::string &level, const std::string &code, const std::string &description)  {
 
 
  //SendSetChunkSize();
  PacketPtr packet = Packet::NewPacket(1024);
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  header->cs_id = kRtmpCSIDAMFIni;
  header->msg_sid = 1;
  header->msg_len = 0;
  header->msg_type = kRtmpMsgTypeAMFMessage;
  packet->SetExt(header);

  char *body = packet->Data();
  char *p = body;

  p += AMFAny::EncodeString(p, "onStatus");
  p += AMFAny::EncodeNumber(p, 0);//csid
  *p ++ = kAMFNull;
  *p ++ = kAMFObject;
  p += AMFAny::EncodeNamedString(p, "level", level);//state error warning
  p += AMFAny::EncodeNamedString(p, "code", code);
  p += AMFAny::EncodeNamedString(p, "description", description);
  *p ++ = 0x00;
  *p ++ = 0x00;
  *p ++ = 0x09;

  header->msg_len = p - body;
  packet->SetPacketSize(header->msg_len);
  RTMP_TRACE << "send status level:" << level << " code:" << code << " description:" << description << " len:" << header->msg_len << "host:" << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::SendPlay()  {
 
  //SendSetChunkSize();
  PacketPtr packet = Packet::NewPacket(1024);
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  header->cs_id = kRtmpCSIDAMFIni;
  header->msg_sid = 1;
  header->msg_len = 0;
  header->msg_type = kRtmpMsgTypeAMFMessage;
  packet->SetExt(header);

  char *body = packet->Data();
  char *p = body;
  p += AMFAny::EncodeString(p, "play");
  p += AMFAny::EncodeNumber(p, 0);
  *p ++ = kAMFNull;
  p += AMFAny::EncodeString(p, name_);//流名称
  p += AMFAny::EncodeNumber(p, -1000.0);//播放起始时间(因为是直播所以是负数)

  header->msg_len = p - body;
  packet->SetPacketSize(header->msg_len);
  RTMP_TRACE << "send play name:" << name_ 
             << " len:" << header->msg_len << "host:" << connection_->PeerAddr().ToIpPort() << " ";
  PushOutQueue(std::move(packet));
}
 
void RtmpContext::HandlePlay(AMFObject &obj)  {
 
  auto tran_id = obj.Property(2)->Number();
  name_ = obj.Property(3)->String();
  ParseNameAndTcUrl();
  RTMP_TRACE << "recv play session_name:" << session_name_ 
             << " param:" << param_ << " host:" << connection_->PeerAddr().ToIpPort();
  is_player_ = true;
  SendUserCtrlMessage(kRtmpEventTypeStreamBegin, 1, 0);
  SendStatus("status", "NetStream.Play.Start", "Start Playing");
  if(rtmp_handler_)
  {
    rtmp_handler_->OnPlay(connection_, session_name_, param_);
  }
}
 
void RtmpContext::ParseNameAndTcUrl()  {
 
  auto pos = app_.find_last_of("/");
  if(pos != std::string::npos)
  {
    app_ = app_.substr(pos + 1);
  }
  param_.clear();
  pos = name_.find_last_of("?");
  if(pos != std::string::npos)
  {
    param_ = name_.substr(pos + 1);
    name_ = name_.substr(0, pos);
  }
  std::string domain;
  std::vector<std::string> list = base::StringUtils::SplitString(tc_url_, "/");
  if(list.size() == 6)//rtmp://ip/domain:port/app/stream
  {
    domain = list[3];
    app_ = list[4];
    name_ = list[5];
  }
  else if(list.size() == 5)//rtmp://domain:port/app/stream
  {
    domain = list[2];
    app_ = list[3];
    name_ = list[4];
  }

  auto p = domain.find_first_of(":");
  if(p != std::string::npos)
  {
    domain = domain.substr(0, p);//去掉端口号
  }

  session_name_.clear();
  session_name_ += domain;
  session_name_ += "/";
  session_name_ += app_;
  session_name_ += "/";
  session_name_ += name_;

  RTMP_TRACE << "session_name:" << session_name_ 
             << " param:" << param_ << " host:" << connection_->PeerAddr().ToIpPort();

}
 
void RtmpContext::SendPublish()  {
 
  //SendSetChunkSize();
  PacketPtr packet = Packet::NewPacket(1024);
  RtmpMsgHeaderPtr header = std::make_shared<RtmpMsgHeader>();
  header->cs_id = kRtmpCSIDAMFIni;
  header->msg_sid = 1;
  header->msg_len = 0;
  header->msg_type = kRtmpMsgTypeAMFMessage;
  packet->SetExt(header);

  char *body = packet->Data();
  char *p = body;
  p += AMFAny::EncodeString(p, "publish");
  p += AMFAny::EncodeNumber(p, 5);
  *p ++ = kAMFNull;
  p += AMFAny::EncodeString(p, name_);
  p += AMFAny::EncodeString(p, "live");


  header->msg_len = p - body;
  packet->SetPacketSize(header->msg_len);
  RTMP_TRACE << "send publish name:" << name_ 
             << " len:" << header->msg_len << "host:" << connection_->PeerAddr().ToIpPort();
  PushOutQueue(std::move(packet));
}
 
//没用到
void RtmpContext::HandlePublish(AMFObject &obj)  {
 
  auto tran_id = obj.Property(1)->Number();
  name_ = obj.Property(3)->String();
  RTMP_TRACE << "recv publish session:" << session_name_ 
             << " param:" << param_ << " host:" << connection_->PeerAddr().ToIpPort();
  is_player_ = false;
  SendStatus("status", "NetStream.Publish.Start", "Start Publishing");

  if(rtmp_handler_)
  {
    rtmp_handler_->OnPublish(connection_, session_name_, param_);
  }
}
 
void RtmpContext::HandleResult(AMFObject &obj)  {
 
  auto id = obj.Property(1)->Number();
  RTMP_TRACE << "recv result id:" << id << " host: " << connection_->PeerAddr().ToIpPort();
  if(id == 1)
  {
    SendCreateStream();   
  }
  else if(id == 4)
  {
    if(is_player_)
    {
      SendPlay();  
    }
    else
    {
      SendPublish();
    }
  }
}
 
void RtmpContext::HandleError(AMFObject &obj)  {
 
  const std::string &description = obj.Property(3)->Object()->Property("description")->String();
  RTMP_ERROR << "recv error description:" << description << " host:" << connection_->PeerAddr().ToIpPort();
  connection_->ForceClose();
}
 
void RtmpContext::SetPacketType(PacketPtr &packet)  {
 
  if(packet->PacketType() == kRtmpMsgTypeAudio)
  {
    packet->SetPacketType(kPacketTypeAudio);
  }
  else if(packet->PacketType() == kRtmpMsgTypeVideo)
  {
    packet->SetPacketType(kPacketTypeVideo);
  }
  else if(packet->PacketType() == kRtmpMsgTypeMetadata)
  {
    packet->SetPacketType(kPacketTypeMeta);
  }
  else if(packet->PacketType() == kRtmpMsgTypeAMF3Meta)
  {
    packet->SetPacketType(kPacketTypeMeta3);
  }

}
 
void RtmpContext::Play(const std::string &url)  {
 
  is_client_ = true;
  is_player_ = true;
  tc_url_ = url;
  ParseNameAndTcUrl();
}
 
void RtmpContext::Publish(const std::string &url)  {
 
  is_client_ = true;
  is_player_ = false;
  tc_url_ = url;
  ParseNameAndTcUrl();
}
