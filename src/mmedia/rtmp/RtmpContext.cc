#include "RtmpContext.h"
#include "../base/MMediaLog.h"
#include "RtmpHandShake.h"
#include "RtmpHeader.h"
#include "../base/BytesReader.h"
#include "../base/BytesWriter.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
using namespace tmms::mm;
 
RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *handler, bool client)
:handshake_(conn, client), connection_(conn), handler_(handler)  
{
 
}
 
int32_t RtmpContext::Parse(MsgBuffer &buf)  {
  int32_t ret = -1;
  if(state_ == kRtmpHandShake)
  {
    ret = handshake_.HandShake(buf);
    //握手成功
    if(ret == 0)
    {
      state_ = kRtmpMessage;
      if(buf.ReadableBytes() > 0)
      {
        return Parse(buf);
      }
    }
    else if(ret == -1)
    {
      RTMP_ERROR << "rtmp handshake failed\n";
    }
    else if(ret == 2)
    {
      state_ = kRtmpWaitingDone;
    }
  }
  else if(state_ == kRtmpMessage)
  {
    std::cout << "start parse message\n";
    ret = ParseMessage(buf);
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
 
  RTMP_TRACE << "recv message type:" << data->PacketType() << ", len = " << data->PacketSize() << "\n";
}

 
bool RtmpContext::BuildChunk(const PacketPtr &packet, uint32_t timestamp, bool fmt0)  {

  RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();
  if(h)
  {
    out_sending_packets_.emplace_back(packet);
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
          RTMP_ERROR << "rtmp had no enough out header buffer.\n";
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
    return true;
  }
  return false;
}

bool RtmpContext::BuildChunk(PacketPtr &&packet, uint32_t timestamp, bool fmt0)  {

  RtmpMsgHeaderPtr h = packet->Ext<RtmpMsgHeader>();
  if(h)
  {
    out_sending_packets_.emplace_back(std::move(packet));
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

      BufferNodePtr node = std::make_shared<BufferNode>((void *)chunk, size);
      sending_bufs_.emplace_back(std::move(node));
      bytes_parsed += size;
      if(bytes_parsed < h->msg_len)
      {
        //没发完，剩下的都是封装成fmt3发送
        //创建一个header继续发
        if(out_current_ - out_buffer_ >= 4096)
        {
          RTMP_ERROR << "rtmp had no enough out header buffer.\n";
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
    return true;
  }
  return false;
}
 
void RtmpContext::Send()  {
 
  if(sending_)return;
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
    BuildChunk(std::move(packet), 0, false);
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
    if(handler_)
    {
      //发送完毕，激活上层，让上层决定处理
      handler_->OnActive(connection_);
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
 
