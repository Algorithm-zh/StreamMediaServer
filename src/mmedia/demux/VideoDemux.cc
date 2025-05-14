#include "VideoDemux.h"
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/BytesReader.h"
using namespace tmms::mm;
 
int32_t VideoDemux::OnDemux(const char *data, size_t size, std::list<SampleBuf> &outs)  {
  VideoCodecID id = (VideoCodecID)(*data&0x0f);
  codec_id_ = id;
  if(id != kVideoCodecIDAVC) {
    DEMUX_ERROR << " not support codec id " << id;
    return -1;
  }
  return DemuxAVC(data, size, outs);
}
 
int32_t VideoDemux::DemuxAVC(const char *data, size_t size, std::list<SampleBuf> &outs)  {
  uint8_t ftype = (*data&0xf0) >> 4;//帧类型
  if(ftype == 5)
  {
    DEMUX_DEBUG << "ignore info frame";
    return 0;
  }
  uint8_t avc_packet_type = data[1];
  int32_t cst = BytesReader::ReadUint24T(data + 2);
  composition_time_ = cst;
  if(avc_packet_type == 0)//序列头
  {
    return DecodeAVCSeqHeader(data + 5, size - 5, outs);
  }
  else if(avc_packet_type == 1)//nalu
  {
    return DecodeAVCNalu(data + 5, size - 5, outs);
  }
  else
  {
    return 0;
  }
}
 
const char* VideoDemux::FindAnnexbNalu(const char* p, const char* end)  {

  for(p += 2; p + 1 < end; p ++)
  {
    if(*p == 0x01 && *(p - 1) == 0x00 && *(p - 2) == 0x00)
    {
      return p + 1;
    }
  }
  return end;
}
 
int32_t VideoDemux::DecodeAVCNaluAnnexb(const char* data, size_t size, std::list<SampleBuf> &outs)  {

  if(size < 3)
  {
    DEMUX_ERROR << "error annexb bytes:" << size;
    return -1;
  }
  int32_t ret = -1;
  const char *data_end = data + size;
  //查找startcode后的一位
  const char *nalu_start = FindAnnexbNalu(data, data_end);
  while(nalu_start < data_end)
  {
    const char* nalu_next = FindAnnexbNalu(nalu_start + 1, data_end);
    int32_t nalu_size = nalu_next - nalu_start;

    if(nalu_size > size || size <= 0)
    {
      DEMUX_ERROR << "error annexb nalu bytes:" << size << "nalu size:" << nalu_size;
      return -1;
    }
    ret = 0;
    outs.emplace_back(SampleBuf(data, nalu_size));
    //检查nalu的类型
    CheckNaluType(data);
    data += nalu_size;
    size -= nalu_size;
    nalu_start = nalu_next;
  }
  return ret;
}
 
int32_t VideoDemux::DecodeAVCNaluAvcc(const char* data, size_t size, std::list<SampleBuf> &outs)  {

  //将所有的nalu取出
  while(size > 1)
  {
    uint32_t nalu_size = 0;
    if(nalu_unit_length_ == 3)
    {
      nalu_size = BytesReader::ReadUint32T(data);
    }
    else if(nalu_unit_length_ == 1)
    {
      nalu_size = BytesReader::ReadUint16T(data);
    }
    else
    {
      nalu_size = data[0];
    }

    data += nalu_unit_length_ + 1;
    size -= nalu_unit_length_ + 1;
    if(nalu_size > size || size <= 0)
    {
      DEMUX_ERROR << "error avcc nalu bytes:" << size << " nalu size:" << nalu_size;
      return -1;
    }
    outs.emplace_back(SampleBuf(data, nalu_size));
    CheckNaluType(data);
    data += nalu_size;
    size -= nalu_size;
  }
  return 0;
}
 
int32_t VideoDemux::DecodeAVCSeqHeader(const char* data, size_t size, std::list<SampleBuf> &outs)  {
  if(size < 5)
  {
    DEMUX_ERROR << "invalid avc seq header";
    return -1;
  }
  config_version_ = data[0];
  profile_ = data[1];
  profile_com_ = data[2];
  level_ = data[3];

  nalu_unit_length_ = data[4] & 0x03;//这里读出来的小一个字节
  DEMUX_DEBUG << "nalu_unit_length " << nalu_unit_length_;

  data += 5;
  size -= 5;

  //sps pps
  if(size < 3)
  {
    DEMUX_ERROR << "invalid avc seq header. not found sps.";
    return -1;
  }
  int8_t sps_num = data[0] & 0x1F;
  //一般就一个
  if(sps_num != 1)
  {
    DEMUX_ERROR << "more than one sps";
  }
  int16_t sps_length = BytesReader::ReadUint16T(data + 1);
  if(sps_length > 0 && sps_length < size - 3)
  {
    DEMUX_DEBUG << "found sps, bytes " << sps_length;
  }
  else
  {
    DEMUX_ERROR << "invalid sps length:" << sps_length;
  }
  sps_.assign(data + 3, sps_length);

  data += 3;
  size -= 3;
  data += sps_length;//跳过sps开始读pps
  size -= sps_length;
  if(size < 3)
  {
    DEMUX_ERROR << "pss header not found";
    return -1;
  }
  int8_t pps_num = data[0] & 0x1F;
  //一般就一个
  if(pps_num != 1)
  {
    DEMUX_ERROR << "more than one pps";
  }  
  int16_t pps_length = BytesReader::ReadUint16T(data + 1);
  if(pps_length > 0 && pps_length <= size - 3)
  {
    DEMUX_DEBUG << "found pps, bytes " << pps_length;
  }
  else
  {
    DEMUX_ERROR << "invalid pps length:" << pps_length;
  }
  pps_.assign(data + 3, pps_length);
  return 0;
}
 
int32_t VideoDemux::DecodeAVCNalu(const char* data, size_t size, std::list<SampleBuf> &outs)  {
  if(payload_format_ == kPayloadFormatUnknowed)
  {
    //不确定时，尝试解析为avcc
    if(!DecodeAVCNaluAvcc(data, size, outs))
    {
      payload_format_ = kPayloadFormatAvcc;
    }
    else
    {
      if(!DecodeAVCNaluAnnexb(data, size, outs))
      {
        payload_format_ = kPayloadFormatAnnexB;
      }
      else
      {
        DEMUX_ERROR << "payload format error.no found format";
        return -1;
      }
    }

  }
  else if(payload_format_ == kPayloadFormatAvcc)
  {
    return DecodeAVCNaluAvcc(data, size, outs);
  }
  else if(payload_format_ == kPayloadFormatAnnexB)
  {
    return DecodeAVCNaluAnnexb(data, size, outs);
  }
  return 0;
}
 
void VideoDemux::CheckNaluType(const char* data)  {
 
  NaluType type = (NaluType)(data[0]&0x1f);
  if(type == kNaluTypeIDR)
  {
    has_idr_ = true;
  }
  else if(type == kNaluTypeAccessUnitDelimiter)
  {
    has_aud_ = true;
  }
  else if(type == kNaluTypeSubsetSPS || type == kNaluTypePPS)
  {
    has_sps_pps_ = true;
  }
}
 
bool VideoDemux::HasIdr() const {
	return has_idr_;
}
 
bool VideoDemux::HasAud() const {
	return has_aud_;
}
 
bool VideoDemux::HasSpsPps() const {
	return has_sps_pps_;
}
