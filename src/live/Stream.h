#pragma once

#include "live/base/TimeCorrector.h"
#include "live/GopMgr.h"
#include "live/CodecHeader.h"
#include "mmedia/base/Packet.h"
#include "PlayerUser.h"
#include "User.h"
#include <string>
#include <memory>
#include <cstdint>
#include <atomic>
#include <vector>
#include <mutex>

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;
        using UserPtr = std::shared_ptr<User>;
        using PlayerUserPtr = std::shared_ptr<PlayerUser>;
        class Session;
        class Stream
        {
        public:
            Stream(Session &s,const std::string &session_name);

            int64_t ReadyTime() const ;
            int64_t SinceStart() const ;
            bool Timeout();
            int64_t DataTime() const ;
            const std::string &SessionName() const ;
            int32_t StreamVersion() const;
            bool HasMedia() const;
            bool Ready() const;

            void AddPacket(PacketPtr && packet);

            void GetFrames(const PlayerUserPtr &user);

        private:
            bool LocateGop(const PlayerUserPtr &user);
            void SkipFrame(const PlayerUserPtr &user);
            void GetNextFrame(const PlayerUserPtr &user); 

            void SetReady(bool ready);
            int64_t data_coming_time_{0};
            int64_t start_timestamp_{0};
            int64_t ready_time_{0};
            std::atomic<int64_t> stream_time_{0};
            Session &session_;
            std::string session_name_;
            std::atomic<int64_t> frame_index_{-1}; 
            uint32_t packet_buffer_size_{1000};
            std::vector<PacketPtr> packet_buffer_;
            bool has_audio_{false};
            bool has_video_{false};
            bool has_meta_{false};
            bool ready_{false};
            std::atomic<int32_t> stream_version_{-1};

            GopMgr gop_mgr_;
            CodecHeader codec_headers_;
            TimeCorrector time_corrector_;
            std::mutex lock_;
        };
    }
}