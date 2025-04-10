#pragma once
#include "mmedia/base/Packet.h"
#include <cstdint>
namespace tmms
{
  namespace live
  {
    using namespace tmms::mm;
    class TimeCorrector  
    {
    const int32_t kMaxVideoDeltaTime = 100;
    const int32_t kMaxAudioDeltaTime = 100;
    const int32_t kDefaultVideoDeltaTime = 40;
    const int32_t kDefaultAudioDeltaTime = 20;
    public:
      TimeCorrector() = default;
      ~TimeCorrector() = default;
      uint32_t CorrectTimestap(const PacketPtr &packet);
      uint32_t CorrectAudioTimeStampByVideo(const PacketPtr &packet); 
      uint32_t CorrectVideoTimeStampByVideo(const PacketPtr &packet);
      uint32_t CorrectAudioTimeStampByAudio(const PacketPtr &packet); 
    private:
      int64_t video_original_timestamp_{-1};
      int64_t video_corrected_timestamp_{0};
      int64_t audio_original_timestamp_{-1};
      int64_t audio_corrected_timestamp_{0};
      int32_t audio_numbers_between_video_{0};
    };
  }
}
