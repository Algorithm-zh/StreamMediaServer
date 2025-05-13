#pragma once

#include <cstdint>
#include <list>
#include <string>
#include "mmedia/base/AVTypes.h"
/*
//AVC Sequence Header
aligned(8) class AVCDecoderConfigurationRecord{
    unsigned int(8) configurationVersion = 1;
    unsigned int(8) AVCProfileIndication;
    unsigned int(8) profile_compatibility;
    unsigned int(8) AVCLevelIndication;
    bit(6) reserved = '111111'b;
    unsigned int(2) lengthSizeMinusOne;//nalu长度用了几个字节，这里减了一
    bit(3) reserved = '111'b;
    unsigned int(5) numOfSequenceParameterSets;
    for(i = 0; i < numOfSequenceParameterSets; i++){
        unsigned int(16) sequenceParameterSetLength;
        bit(8*sequenceParameterSetLength) sequenceParameterSetNALUnit;
    }
    unsigned int(8) numOfPictureParameterSets;
    for(i = 0; i < numOfPictureParameterSets; i++){
        unsigned int(16) pictureParameterSetLength;
        bit(8*pictureParameterSetLength) pictureParameterSetNALUint;
    }
}
*/

namespace tmms
{
  namespace mm
  {
    enum AVCPayloadFormat
    {
      kPayloadFormatUnknowed = 0,
      kPayloadFormatAvcc = 1,
      kPayloadFormatAnnexB = 2,
    };
    class VideoDemux
    {
    public:
      VideoDemux() = default;
      ~VideoDemux() = default;
      //DEMUX成员函数
      int32_t OnDemux(const char *data, size_t size, std::list<SampleBuf> &outs);
      bool HasIdr() const;
      bool HasAud() const;
      bool HasSpsPps() const;
      VideoCodecID GetCodecID() const { return codec_id_; }
      int32_t GetCST() const { return composition_time_; }
      const std::string &GetSPS() const { return sps_; }
      const std::string &GetPPS() const { return pps_; }
      void Reset()
      {
        has_aud_ = false;
        has_idr_ = false;
        has_sps_pps_ = false;
      }

    private:
      int32_t DemuxAVC(const char *data, size_t size, std::list<SampleBuf> &outs);
      //查找startcode并返回下一位
      const char* FindAnnexbNalu(const char* p, const char* end);
      int32_t DecodeAVCNaluAnnexb(const char* data, size_t size, std::list<SampleBuf> &outs);
      int32_t DecodeAVCNaluAvcc(const char* data, size_t size, std::list<SampleBuf> &outs);
      int32_t DecodeAVCSeqHeader(const char* data, size_t size, std::list<SampleBuf> &outs);
      //入口
      int32_t DecodeAVCNalu(const char* data, size_t size, std::list<SampleBuf> &outs);
      //Nalu Type成员函数
      void CheckNaluType(const char* data);

      VideoCodecID codec_id_;
      int32_t composition_time_{0};//用来纠正时间戳
      uint8_t config_version_{0};
      uint8_t profile_{0};
      uint8_t profile_com_{0};
      uint8_t level_{0};
      uint8_t nalu_unit_length_{0};//nalu所占字节数
      bool avc_ok_{false};
      std::string sps_;
      std::string pps_;
      std::string aud_;
      AVCPayloadFormat payload_format_{kPayloadFormatUnknowed};
      bool has_aud_{false};
      bool has_idr_{false};
      bool has_sps_pps_{false};
      
    };
  }
}
