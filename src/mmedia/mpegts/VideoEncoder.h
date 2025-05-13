#pragma once
#include "mmedia/mpegts/StreamWriter.h"
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/Packet.h"
#include "mmedia/demux/VideoDemux.h"
#include <list>
#include <cstdint>

namespace tmms
{
  namespace mm
  {
    class VideoEncoder
    {
    public:
       VideoEncoder() = default;
      ~VideoEncoder() = default;
      int32_t EncodeVideo(StreamWriter *writer, bool key, PacketPtr &data, int64_t dts);
      //其它成员函数
      void SetPid(uint16_t pid);
      void SetStreamType(TsStreamType type);
    
    private:
      int32_t EncodeAvc(StreamWriter *writer, std::list<SampleBuf> &sample_list, bool key, int64_t pts);
      int32_t AvcInsertStartCode(std::list<SampleBuf> &sample_list);
      int32_t WriteVideoPes(StreamWriter *writer, std::list<SampleBuf> &result, int payload_size, int64_t pts, int64_t dts, int key);


      uint16_t pid_{0xe000};
      TsStreamType type_{kTsStreamReserved};
      int8_t cc_{-1};
      bool startcode_inserted_{false};//是否是第一次插入
      bool sps_pps_appended_{false};
      VideoDemux demux_;    
    };
  }
}
