#pragma once
#include "User.h"
#include "base/TimeCorrector.h"
#include "mmedia/base/Packet.h"
#include "live/base/TimeCorrector.h"
#include <cstdint>
#include <vector>
namespace tmms
{
  namespace live
  {
    using namespace tmms::mm;
    class PlayerUser : public User
    {
    public:
      friend class Stream;
      //委托构造
      using User::User;
      //CodecHeader成员函数
      PacketPtr Meta() const;
      PacketPtr AudioHeader() const;
      PacketPtr VideoHeader() const;
      void ClearMeta();
      void ClearAudioHeader();
      void ClearVideoHeader();
      //其它成员函数
      virtual bool PostFrames() = 0;
      TimeCorrector& GetTimeCorrector();
   protected:
      PacketPtr meta_;
      PacketPtr audio_header_;
      PacketPtr video_header_;
      
      bool wait_meta_{true};//需不需要meta
      bool wait_audio_{true};
      bool wait_video_{true};
      //判断跳帧后需不需要发送头 
      int32_t video_header_index_{0};
      int32_t audio_header_index_{0};
      int32_t meta_index_{0};

      TimeCorrector time_corrector_;
      bool wait_timeout_{false};
      int32_t out_version_{-1};
      int32_t out_frame_timestamp_{0};
      std::vector<PacketPtr> out_frames_;
      int32_t out_index_{-1};//sending frame index
      
    };
  }
}
