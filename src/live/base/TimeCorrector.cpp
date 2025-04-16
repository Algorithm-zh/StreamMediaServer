#include "TimeCorrector.h"
#include "CodecUtils.h"
#include "live/base/LiveLog.h"
using namespace tmms::live;

uint32_t TimeCorrector::CorrectTimestamp(const PacketPtr &packet)
{
    if(!CodecUtils::IsCodecHeader(packet))
    {
        //LIVE_TRACE << "ts:" << packet->TimeStamp() << " size:" << packet->PacketSize();
        int32_t pt = packet->PacketType();
        if(pt == kPacketTypeVideo)
        {
            return CorrectVideoTimeStampByVideo(packet);
        }
        else if(pt == kPacketTypeAudio)
        {
            return CorrectAudioTimeStampByVideo(packet);
        }
    }
    return 0;
}
uint32_t TimeCorrector::CorrectAudioTimeStampByVideo(const PacketPtr &packet)
{
    ++audio_numbers_between_video_;
    if(audio_numbers_between_video_ > 1)
    {
        return CorrectAudioTimeStampByAudio(packet);
    }

    int64_t time = packet->TimeStamp();
    if(video_original_timestamp_ == -1)
    {
        audio_original_timestamp_ = time;
        audio_corrected_timestamp_ = time;
        return time;
    }

    int64_t delta = time - video_original_timestamp_;
    bool fine = (delta>-kMaxVideoDeltaTime)&&(delta<kMaxVideoDeltaTime);
    if(!fine)
    {
        delta = kDefaultVideoDeltaTime;
    }

    audio_original_timestamp_ = time;
    audio_corrected_timestamp_ = video_corrected_timestamp_ + delta;
    if(audio_corrected_timestamp_ < 0)
    {
        audio_corrected_timestamp_ = 0;
    }
    return audio_corrected_timestamp_;
}
uint32_t TimeCorrector::CorrectVideoTimeStampByVideo(const PacketPtr &packet)
{
    audio_numbers_between_video_ = 0;
    int64_t time = packet->TimeStamp();
    if(video_original_timestamp_ == -1)
    {
        video_original_timestamp_ = time;
        video_corrected_timestamp_ = time;

        if(audio_original_timestamp_ != -1)
        {
            int32_t delta = audio_original_timestamp_ - video_original_timestamp_;
            if(delta<=-kMaxVideoDeltaTime||delta >=kMaxVideoDeltaTime)
            {
                video_original_timestamp_ = audio_original_timestamp_;
                video_corrected_timestamp_ = audio_corrected_timestamp_;
            }
        }
    }

    int64_t delta = time - video_original_timestamp_;
    bool fine = (delta>-kMaxVideoDeltaTime)&&(delta<kMaxVideoDeltaTime);
    if(!fine)
    {
        delta = kDefaultVideoDeltaTime;
    }

    video_original_timestamp_ = time;
    video_corrected_timestamp_ += delta;
    if(video_corrected_timestamp_ < 0)
    {
        video_corrected_timestamp_ = 0;
    }
    return video_corrected_timestamp_;
}
uint32_t TimeCorrector::CorrectAudioTimeStampByAudio(const PacketPtr &packet)
{
    int64_t time = packet->TimeStamp();

    if(audio_original_timestamp_ == -1)
    {
        audio_original_timestamp_ = time;
        audio_corrected_timestamp_ = time;
        return time;
    }

    int64_t delta = time - audio_original_timestamp_;
    bool fine = (delta>-kMaxAudioDeltaTime)&&(delta<kMaxAudioDeltaTime);
    if(!fine)
    {
        delta = kDefaultAudioDeltaTime;
    }

    audio_original_timestamp_ = time;
    audio_corrected_timestamp_ += delta;
    if(audio_corrected_timestamp_ < 0)
    {
        audio_corrected_timestamp_ = 0;
    }
    return audio_corrected_timestamp_;
}