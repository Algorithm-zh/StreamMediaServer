#include "RtmpContext.h"
#include "../base/MMediaLog.h"
#include "RtmpHandShake.h"
#include "RtmpHeader.h"
#include "../base/BytesReader.h"
#include <algorithm>
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


