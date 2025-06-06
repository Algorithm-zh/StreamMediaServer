#include "mmedia/mpegts/VideoEncoder.h"
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/MMediaLog.h"
#include "TsTool.h"
#include <cstdint>
#include <sys/types.h>
using namespace tmms::mm;

namespace
{
  uint8_t *get_start_payload(uint8_t *pkt)
  {
    //如果有适配域，则返回适配域的开始
    if(pkt[3] & 0x20)//pkt[3]是adaptation_field_control
    {
      //这里加5是因为有适配域，所以要加上适配域的长度
      return pkt + 5 + pkt[4];
    }
    else
    {
      return pkt + 4;
    }
  }
  //srs中的算法
  int WritePcr(uint8_t *buf, int64_t pcr) {
    int64_t pcrv = (0) & 0x1ff;
    pcrv |= (0x3f << 9) & 0x7E00;
    pcrv |= ((pcr) << 15) & 0xFFFFFFFF8000LL;

    char *pp = (char*)&pcrv;
    *buf++ = pp[5];
    *buf++ = pp[4];
    *buf++ = pp[3];
    *buf++ = pp[2];
    *buf++ = pp[1];
    *buf++ = pp[0];

    return 6;
  }
}
 
int32_t VideoEncoder::EncodeVideo(StreamWriter *writer, bool key, PacketPtr &data, int64_t dts)  {

  std::list<SampleBuf> list;
  auto ret = demux_.OnDemux(data->Data(), data->PacketSize(), list);
  if(ret == -1)
  {
    MPEGTS_ERROR << "video demux error";
    return -1;
  }
  if(TsTool::IsCodecHeader(data))
  {
      return 0;
  }
  writer->AppendTimeStamp(dts);
  dts = dts * 90;
  if(demux_.GetCodecID() == kVideoCodecIDAVC)
  {
    return EncodeAvc(writer, list, key, dts);
  }
	return 0;
}
 
int32_t VideoEncoder::EncodeAvc(StreamWriter *writer, std::list<SampleBuf> &sample_list, bool key, int64_t dts)  {

  int32_t total_size = 0;
  std::list<SampleBuf> result;
  bool startcode_inserted = true;
  if(!demux_.HasAud())
  {
    static uint8_t default_aud_nalu[] = {0x09, 0xf0};
    static SampleBuf defalut_aud_buf((const char *)&default_aud_nalu[0], 2);
    total_size += AvcInsertStartCode(result, startcode_inserted);//before aud insert startcode
    result.push_back(defalut_aud_buf);
    total_size += 2;
  }
  for(auto const &l : sample_list)
  {
    if(l.size <= 0)
    {
      MPEGTS_ERROR << "invalid avc frame length";
      continue;
    }
    auto bytes = l.addr;
    NaluType type = (NaluType)(bytes[0] & 0x1f);
    //spspps不同说明是新的切片，需要写新的spspps
    if(type == kNaluTypeIDR&&!demux_.HasSpsPps() &&
      (writer->GetSPS() != demux_.GetSPS() ||
       writer->GetPPS() != demux_.GetPPS() ||
       !writer->GetSpsPpsAppended()))
    {
      //这个操作很关键，一定要用引用，否则sps的值就会被改变!!!
      auto const &sps = demux_.GetSPS();
      if(!sps.empty())
      {
        total_size += AvcInsertStartCode(result, startcode_inserted);
        //这里，这里就是把地址填进去的，而sps如果不用引用，那么一旦出了作用域，data的值就会成为其他的东西
        result.emplace_back(sps.data(), sps.size());
        total_size += sps.size();
        writer->SetSPS(sps);
      }
      else
      {
        MPEGTS_ERROR << "no sps";
      }

      auto const &pps = demux_.GetPPS();
      if(!pps.empty())
      {
        total_size += AvcInsertStartCode(result, startcode_inserted);
        result.emplace_back(pps.data(), pps.size());
        total_size += pps.size();
        writer->SetPPS(pps);
      }
      else
      {
        MPEGTS_ERROR << "no pps";
      }
      writer->SetSpsPpsAppended(true);       
    }
    
    total_size += AvcInsertStartCode(result, startcode_inserted);
    result.emplace_back(l.addr, l.size);
    total_size += l.size;
  }
  int64_t pts = dts;
  if(demux_.GetCST() > 0)
  {
    pts = dts + demux_.GetCST() * 90;
  }

  return WriteVideoPes(writer, result, total_size, pts, dts, key);
}
 
int32_t VideoEncoder::AvcInsertStartCode(std::list<SampleBuf> &sample_list ,bool &startcode_inserted)  {

  //是否是第一次插入
  if(startcode_inserted)
  {
    static uint8_t default_start_nalu[] = {0x00, 0x00, 0x01};
    static SampleBuf defalut_start_buf((const char *)&default_start_nalu[0], 3);
    sample_list.emplace_back(defalut_start_buf);
    return 3;
  }
  else
  {
    //第一次的话是四个字节
    static uint8_t default_start_nalu[] = {0x00, 0x00, 0x00, 0x01};
    static SampleBuf defalut_start_buf((const char *)&default_start_nalu[0], 4);
    sample_list.emplace_back(defalut_start_buf);
    startcode_inserted = true;
    return 4;
  }
}
 
int32_t VideoEncoder::WriteVideoPes(StreamWriter *writer, std::list<SampleBuf> &result, int payload_size, int64_t pts, int64_t dts, bool key)  {
  //视频和音频不一样的地方是视频要写pcr
  
  uint8_t buf[188], *q;
  int32_t val = 0;
  bool is_start = true;
  //构造ts包
  while(payload_size > 0 && !result.empty())
  {
    memset(buf, 0x00, 188);
    q = buf;
    //sync_byte
    *q ++ = 0x47;
    val = pid_ >> 8;
    if(is_start)
    {
      val |= 0x40;
    }
    *q ++ = val;
    *q ++ = pid_;
    cc_ = (cc_ + 1) & 0xf;
    *q ++ = 0x10 | cc_;
    //data_byte
    if(is_start)
    {
      //关键帧第一个帧需要先发pcr
      //pcr是写在适配域里的
      if(key)
      {
        buf[3] |= 0x20;//启用adaption field
        buf[4] = 1;//adaption_field_length先设为1(flga占一字节)
        buf[5] = 0x10;//设置pcr_flag
        q = get_start_payload(buf);
        auto size = WritePcr(q, pts);
        buf[4] += size;//更新adaption_field_length
        q = get_start_payload(buf);//重新获取payload起始位置
      }
      //pes startcode3B
      *q ++ = 0x00;
      *q ++ = 0x00;
      *q ++ = 0x01;
      //stream id1B
      *q ++ = 0xe0;//video id
      
      int16_t header_len = 5;
      u_int8_t flags = 0x02;

      if(pts != dts)
      {
        //pts和dts不一样，则需要加上pts
        header_len += 5;
        flags = 0x03;
      }

      int32_t len = payload_size + header_len + 3;//5 = pts长度。3 = pes可选字段长度
      if(len > 0xffff)
      {
        len = 0;
      }
      //pes packet length2B
      *q ++ = len >> 8;
      *q ++ = len;
      //flag1B
      *q ++ = 0x80;//0x80表示只含有pts,0xc0则都有
      *q ++ = flags << 6;// PTS 只有有效（'10'）的标志
      //pes data length1B
      *q ++ = header_len;//可选字段长度
      if(flags == 0x02)
      {
        TsTool::WritePts(q, 0x02, pts);
        q += 5;
      }
      else if(flags == 0x03)
      {
        TsTool::WritePts(q, 0x03, pts);
        q += 5;
        TsTool::WritePts(q, 0x01, dts);
        q += 5;
      }

      is_start = false;
    }
    //计算还剩多少空间写payload,如果剩余空间多于payload,则需要填充stuffing
    int32_t header_len = q - buf;
    int32_t len = 188 - header_len;
    if(len > payload_size)
    {
      len = payload_size;
    }
    //stuffing（填充部分）,每个ts包都要为188字节
    int32_t stuffing = 188 - header_len - len;
    if(stuffing > 0)
    {
      if(buf[3] & 0x20)//已有adaptation_field，扩展
      {
        int32_t af_len = buf[4] + 1;
        //把原本payload数据向后移动stuffing字节
        memmove(buf + 4 + af_len + stuffing, buf + 4 + af_len, header_len - (4 + af_len));
        buf[4] += stuffing;
        memset(buf + 4 + af_len, 0xff, stuffing);
      }
      else //没有adaptation_field，新增
      {
        memmove(buf + 4 + stuffing, buf + 4, header_len - 4);
        buf[3] |= 0x20;//设置adaptation_field标志位,表示有adaptation_field了 
        buf[4] = stuffing - 1;
        if(stuffing > 2)
        {
          buf[5] = 0x00;
          memset(buf + 6, 0xff, stuffing - 2);
        }
      }
    }
    auto slen = len;
    //把音频数据写进ts包
    while(slen > 0 && !result.empty())
    {
      auto & sbuf = result.front();
      if(sbuf.size <= slen)
      {
        memcpy(buf + 188 - slen, sbuf.addr, sbuf.size);
        slen -= sbuf.size;
        result.pop_front();
      }
      else
      {
        //ts包写满了，只能写部分数据
        memcpy(buf + 188 - slen, sbuf.addr, slen);
        sbuf.addr += slen;
        sbuf.size -= slen;
        slen = 0;
        break;
      }
    }
    payload_size -= len;
    writer->Write(buf, 188);
  }
  return 0;
}
 
void VideoEncoder::SetPid(uint16_t pid)  {
  pid_ = pid;
}
 
void VideoEncoder::SetStreamType(TsStreamType type)  {
  type_ = type;
}
