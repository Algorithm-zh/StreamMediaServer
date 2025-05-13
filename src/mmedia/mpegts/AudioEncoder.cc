#include "AudioEncoder.h"
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/mpegts/TsTool.h"
#include <cstdint>
#include <cstring>
using namespace tmms::mm;
 
namespace
{
  AacProfile AACObjectTypeToAacProfile(AACObjectType type)
  {
    switch (type) 
    {
    case kAACObjectTypeAacMain:
      return AacProfileMain;
    case kAACObjectTypeAacHE:
    case kAACObjectTypeAacHEV2:
    case kAACObjectTypeAacLC:
      return AacProfileLC;
    case kAACObjectTypeAacSSR:
      return AacProfileSSR;
    default:
      return AacProfileReserved;
    }
  }
}

//解封装flv的audio tag 并重新封装(如果是aac类型的，其实就是将ADIF类型转为ADTS类型)
int32_t AudioEncoder::EncodeAudio(StreamWriter *writer, PacketPtr &data, int64_t dts)  {
  std::list<SampleBuf> list;
  auto ret = demux_.OnDemux(data->Data(), data->PacketSize(), list);
  if(ret == -1)
  {
    MPEGTS_ERROR << "demux audio data failed";
    return -1;
  }
  if(TsTool::IsCodecHeader(data))
  {
    return 0;
  }
  writer->AppendTimeStamp(dts);
  dts = dts * 90;//mpegts用的系统时钟是90kHz, 比如视频帧解码时间位1000ms,编码成pes需要转换为90000
  if(demux_.GetCodecId() == kAudioCodecIDAAC)
  {
    return EncodeAAC(writer, list, dts);
  }
  else if(demux_.GetCodecId() == kAudioCodecIDMP3)
  {
    return EncodeMP3(writer, list, dts);
  }
  return 0;
}
 
//在每个aac包前面加上adts头并写入pes
int32_t AudioEncoder::EncodeAAC(StreamWriter *writer, std::list<SampleBuf> &sample_list, int64_t pts)  {
  for(auto const & l : sample_list)
  {
    if(l.size <= 0 || l.size > 0x1fff)
    {
      MPEGTS_ERROR << "aac data size error";
      return -1;
    }
    int32_t frame_length = 7 + l.size;
    std::list<SampleBuf> result;
    //adts_fixed_header28
    //syncword12 ID1 layer2 protection_absent1 
    uint8_t adts_header[7] = {0xff, 0xf9, 0x00, 0x00, 0x00, 0x0f, 0xfc};//初始化adts头
    AacProfile profile = AACObjectTypeToAacProfile(demux_.GetObjectType());
    //profile_ObjectType2
    adts_header[2] = (profile << 6) & 0xc0;//1100
    //sampling_frequency_index4 private_bit1
    adts_header[2] |= (demux_.GetSampleRateIndex() << 2) & 0x3c; //0011 1100
    //channel_configuration3
    adts_header[2] |= (demux_.GetChannel() >> 2) & 0x01;//0001
    adts_header[3] = (demux_.GetChannel() << 6) & 0xc0;//1100

    //adts_variable_header28
    //copyright2 aac_frame_length13
    adts_header[3] |= (frame_length >> 11) & 0x03;//frame_length高2位0011
    adts_header[4] = (frame_length >> 3) & 0xff;//中8位
    adts_header[5] = (frame_length << 5) & 0xe0;//低三位

    adts_header[5] |= 0x1f;
    result.emplace_back(SampleBuf((const char*)adts_header, 7));
    result.emplace_back(l.addr, l.size);

    return WriteAudioPes(writer, result, 7 + l.size, pts);
  }
  return 0;
}
 
//mp3不需要转,直接写入pes
int32_t AudioEncoder::EncodeMP3(StreamWriter *writer, std::list<SampleBuf> &sample_list, int64_t pts)  {
  int32_t size = 0;
  for(auto const & l : sample_list)
  {
    size += l.size;
  }
  return WriteAudioPes(writer, sample_list, size, pts);
}
 
int32_t AudioEncoder::WriteAudioPes(StreamWriter *writer, std::list<SampleBuf> &result, int payload_size, int64_t dts)  {
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
      //pes startcode3B
      *q ++ = 0x00;
      *q ++ = 0x00;
      *q ++ = 0x01;
      //stream id1B
      *q ++ = 0xc0;//audio id
      int32_t len = payload_size + 5 + 3;//5 = pts长度。3 = pes可选字段长度
      if(len > 0xffff)
      {
        len = 0;
      }
      //pes packet length2B
      *q ++ = len >> 8;
      *q ++ = len;
      //flag1B
      *q ++ = 0x80;//0x80表示只含有pts,0xc0则都有
      *q ++ = 0x02 << 6;// PTS 只有有效（'10'）的标志
      //pes data length1B
      *q ++ = 5;//可选字段长度=pts占5个字节
      //pts5B
      TsTool::WritePts(q, 0x02, dts);
      q += 5;

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
 
void AudioEncoder::SetPid(uint16_t pid)  {
  pid_ = pid;
}
 
void AudioEncoder::SetStreamType(TsStreamType type)  {
  type_ = type;
}
