#include "Stream.h"
#include "base/TTime.h"
#include "live/base/CodecUtils.h"
#include "live/base/LiveLog.h"
#include "Session.h"

using namespace tmms::live;
using namespace tmms::base;
Stream::Stream(Session& s,const std::string &session_name)
:session_(s),session_name_(session_name),packet_buffer_(packet_buffer_size_)
{
    stream_time_ = TTime::NowMS();
    start_timestamp_ = TTime::NowMS();
}

int64_t Stream::ReadyTime() const 
{
    return ready_time_;
}
int64_t Stream::SinceStart() const 
{
    return TTime::NowMS() - start_timestamp_;
}
bool Stream::Timeout()
{
    auto delta = TTime::NowMS() - stream_time_;
    if(delta > 20 * 1000)
    {
        return true;
    }
    return false;
}
int64_t Stream::DataTime() const 
{
    return data_coming_time_;
}

const std::string &Stream::SessionName() const
{
    return session_name_;
}
int32_t Stream::StreamVersion() const
{
    return stream_version_;
}
bool Stream::HasMedia() const
{
    return has_audio_||has_video_||has_meta_;
}
bool Stream::Ready() const
{
    return ready_;
}
void Stream::SetReady(bool ready)
{
    ready_ = true;
    ready_time_ = TTime::NowMS();
}

void Stream::AddPacket(PacketPtr && packet)
{
    auto t = time_corrector_.CorrectTimestamp(packet);
    packet->SetTimeStamp(t);

    {
        std::lock_guard<std::mutex> lk(lock_);
        auto index = ++frame_index_;
        packet->SetIndex(index);
        if(packet->IsVideo()&&CodecUtils::IsKeyFrame(packet))
        {
            SetReady(true);
            packet->SetPacketType(kPacketTypeVideo|kFrameTypeKeyFrame);
        }

        if(CodecUtils::IsCodecHeader(packet))
        {
            codec_headers_.ParseCodecHeader(packet);
            if(packet->IsVideo())
            {
                has_video_ = true;
                stream_version_++;
            }
            else if(packet->IsAudio())
            {
                has_audio_ = true;
                stream_version_++;
            }
            else if(packet->IsMeta())
            {
                has_meta_ = true;
                stream_version_++;
            }
        }

        gop_mgr_.AddFrame(packet);
        packet_buffer_[index%packet_buffer_size_] = std::move(packet);
        auto min_idx = frame_index_ - packet_buffer_size_;
        if(min_idx>0)
        {
            gop_mgr_.ClearExpriedGop(min_idx);
        }
    }

    if(data_coming_time_ == 0)
    {
        data_coming_time_ = TTime::NowMS();
    }

    stream_time_ = TTime::NowMS();
    auto frame = frame_index_.load();
    if(frame<300||frame%5==0)
    {
        session_.ActiveAllPlayers();
    }
}

void Stream::GetFrames(const PlayerUserPtr &user)
{
    if(!HasMedia())
    {
        return ;
    }
    if(user->meta_
        ||user->audio_header_
        ||user->video_header_
        ||!user->out_frames_.empty())
    {
        return ;
    }
    std::lock_guard<std::mutex> lk(lock_);
    if(user->out_index_>=0)
    {
        int min_idx = frame_index_ - packet_buffer_size_;
        int content_lantency = user->GetAppInfo()->content_latency;
        if((user->out_index_<min_idx)
            ||((gop_mgr_.LastestTimeStamp() - user->out_frame_timestamp_)>2*content_lantency))
        {
            LIVE_INFO << "need skip out index:" << user->out_index_
                    << ",min idx:" << min_idx
                    << ",out timestamp:" << user->out_frame_timestamp_
                    << ",latest timestamp:" << gop_mgr_.LastestTimeStamp();
            SkipFrame(user);
        }    
    }
    else
    {
        if(!LocateGop(user))
        {
            return ;
        }
    }
    GetNextFrame(user);
}
bool Stream::LocateGop(const PlayerUserPtr &user)
{
    int content_lantency = user->GetAppInfo()->content_latency;
    int lantency = 0;
    auto idx = gop_mgr_.GetGopByLatency(content_lantency,lantency);
    if(idx != -1)
    {
        user->out_index_ = idx - 1;
    }
    else 
    {
        auto elapsed = user->ElapsedTime();
        if(elapsed>=1000&&!user->wait_timeout_)
        {
            LIVE_DEBUG << "wait Gop keyframe timeout. host:" << user->user_id_;
            user->wait_timeout_ = true;
        }
        return false;
    }

    user->wait_meta_ = (user->wait_meta_&&has_meta_);
    if(user->wait_meta_)
    {
        auto meta = codec_headers_.Meta(idx);
        if(meta)
        {
            user->wait_meta_ = false;
            user->meta_ = meta;
            user->meta_index_ = meta->Index();
        }
    }
    user->wait_audio_ = (user->wait_audio_&&has_audio_);
    if(user->wait_audio_)
    {
        auto audio_header = codec_headers_.AudioHeader(idx);
        if(audio_header)
        {
            user->wait_audio_= false;
            user->audio_header_ = audio_header;
            user->audio_header_index_ = audio_header->Index();
        }
    }
    user->wait_video_ = (user->wait_video_&&has_video_);
    if(user->wait_video_)
    {
        auto video_header = codec_headers_.VideoHeader(idx);
        if(video_header)
        {
            user->wait_video_ = false;
            user->video_header_ = video_header;
            user->video_header_index_ = video_header->Index();
        }
    }     

    if(user->wait_meta_||user->wait_audio_||user->wait_video_||idx == -1)
    {
        auto elapsed = user->ElapsedTime();
        if(elapsed>=1000&&!user->wait_timeout_)
        {
            LIVE_DEBUG << "wait Gop keyframe timeout elapsed:" << elapsed 
                    << "ms,frame index:" << frame_index_
                    << ",gop size:" << gop_mgr_.GopSize()
                    <<". host:" << user->user_id_;
            user->wait_timeout_ = true;
        }
        return false;
    }   

    user->wait_meta_ = true;
    user->wait_audio_ = true;
    user->wait_video_ = true;
    user->out_version_ = stream_version_;

    auto elapsed = user->ElapsedTime();

    LIVE_DEBUG << "locate GOP sucess.elapsed:" << elapsed 
                << "ms,gop idx:" << idx
                << ",frame index:" << frame_index_
                << ",lantency:" << lantency
                << ",user:" << user->user_id_;
    return true;

}
void Stream::SkipFrame(const PlayerUserPtr &user)
{
    int content_lantency = user->GetAppInfo()->content_latency;
    int lantency = 0;
    auto idx = gop_mgr_.GetGopByLatency(content_lantency,lantency);
    if(idx == -1 || idx <= user->out_index_)
    {
        return;
    }

    auto meta = codec_headers_.Meta(idx);
    if(meta)
    {
        if(meta->Index()>user->meta_index_)
        {
            user->meta_ = meta;
            user->meta_index_ = meta->Index();
        }
    }
    auto audio_header = codec_headers_.AudioHeader(idx);
    if(audio_header)
    {
        if(audio_header->Index()>user->audio_header_index_)
        {
            user->audio_header_ = audio_header;
            user->audio_header_index_ = audio_header->Index();
        }
    }
    auto video_header = codec_headers_.VideoHeader(idx);
    if(video_header)
    {
        if(video_header->Index()>user->video_header_index_)
        {
            user->video_header_ = video_header;
            user->video_header_index_ = video_header->Index();
        }
    }      

    LIVE_DEBUG << "skip frame " << user->out_index_ << "->" << idx
                << ",lantency:" << lantency 
                << ",frame_index:" << frame_index_
                << ",host:" << user->user_id_;
    user->out_index_ = idx - 1;  
}

void Stream::GetNextFrame(const PlayerUserPtr &user)
{
    auto idx = user->out_index_ + 1;
    auto max_idx = frame_index_.load();
    for(int i = 0;i < 10;i++)
    {
        if(idx>max_idx)
        {
            break;
        }
        auto &pkt = packet_buffer_[idx%packet_buffer_size_];
        if(pkt)
        {
            user->out_frames_.emplace_back(pkt);
            user->out_index_ = pkt->Index();
            user->out_frame_timestamp_ = pkt->TimeStamp();
            idx = pkt->Index() + 1;
        }
        else 
        {
            break;
        }
    }
}