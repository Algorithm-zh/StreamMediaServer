#pragma once

#include "User.h"
#include "mmedia/base/Packet.h"
#include "live/base/TimeCorrector.h"
#include <vector>

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;

        class PlayerUser:public User
        {
        public:
            friend class Stream;
            explicit PlayerUser(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s);

            PacketPtr Meta() const;
            PacketPtr VideoHeader() const;
            PacketPtr AudioHeader() const;
            void ClearMeta();
            void ClearAudioHeader();
            void ClearVideoHeader();  

            virtual bool PostFrames() = 0;
            TimeCorrector& GetTimeCorrector();
        protected:
            PacketPtr video_header_;   
            PacketPtr audio_header_;  
            PacketPtr meta_;  

            bool wait_meta_{true}; 
            bool wait_audio_{true}; 
            bool wait_video_{true}; 

            int32_t video_header_index_{0};
            int32_t audio_header_index_{0};
            int32_t meta_index_{0};

            TimeCorrector time_corrector_;
            bool wait_timeout_{false};
            int32_t out_version_{-1};
            int32_t out_frame_timestamp_{0};
            std::vector<PacketPtr> out_frames_;
            int32_t out_index_{-1};
        };
    }
}