#pragma once
#include <cstdint>
#include <list>
#include "PSIWriter.h"
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/Packet.h"
#include "mmedia/demux/AudioDemux.h"
#include "mmedia/mpegts/StreamWriter.h"
namespace tmms
{
  namespace mm
  {

    class AudioEncoder
    {
    public:
      AudioEncoder() = default;
      ~AudioEncoder() = default;
      //pes封装成员函数
      int32_t EncodeAudio(StreamWriter *writer, PacketPtr &data, int64_t dts);
      //其它成员函数
      void SetPid(uint16_t pid);
      void SetStreamType(TsStreamType type);
    
    private:
      int32_t EncodeAAC(StreamWriter *writer, std::list<SampleBuf> &sample_list, int64_t pts);
      int32_t EncodeMP3(StreamWriter *writer, std::list<SampleBuf> &sample_list, int64_t pts);
      //原始数据加头后调用这个写成pes输出
      int32_t WriteAudioPes(StreamWriter *writer, std::list<SampleBuf> &result, int payload_size, int64_t dts);


      uint16_t pid_{0xe000};
      TsStreamType type_{kTsStreamReserved};
      int8_t cc_{-1};
      AudioDemux demux_;
    };
  }
}
