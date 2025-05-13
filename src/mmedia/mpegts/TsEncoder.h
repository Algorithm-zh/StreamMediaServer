#pragma once
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/Packet.h"
#include "mmedia/mpegts/AudioEncoder.h"
#include "mmedia/mpegts/StreamWriter.h"
#include "mmedia/mpegts/PatWriter.h"
#include "mmedia/mpegts/PmtWriter.h"
#include "mmedia/mpegts/VideoEncoder.h"
#include <cstdint>
#include <sys/types.h>
namespace tmms
{
  namespace mm
  {
    class TsEncoder
    {
    public:
      TsEncoder() = default;
      ~TsEncoder() = default;
      int32_t Encode(StreamWriter *writer, PacketPtr &data, int64_t dts);
      void SetStreamType(StreamWriter *w, VideoCodecID vc, AudioCodecID ac);
      int32_t WritePatPmt(StreamWriter *w);
    private:
      PatWriter pat_writer_;
      PmtWriter pmt_writer_;
      AudioEncoder audio_encoder_;
      VideoEncoder video_encoder_;
      TsStreamType video_type_{kTsStreamReserved};
      TsStreamType audio_type_{kTsStreamReserved};
      uint16_t video_pid_{0xe000};
      uint16_t audio_pid_{0xe000};
    };
  }
}
