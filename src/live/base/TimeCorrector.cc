#include "TimeCorrector.h"
#include "CodecUtils.h"
using namespace tmms::live;


 
uint32_t TimeCorrector::CorrectTimestamp(const PacketPtr &packet)  {
  //如果是音视频的序列头，直接返回0
  if(CodecUtils::IsCodecHeader(packet))
  {
    return 0;
  }
  if(packet->IsVideo())
  {
    return CorrectVideoTimeStampByVideo(packet);
  }
  else if(packet->IsAudio())
  {
    return CorrectAudioTimeStampByAudio(packet);
  }
  return 0;
}
uint32_t TimeCorrector::CorrectAudioTimeStampByVideo(const PacketPtr &packet)  {

  ++audio_numbers_between_video_;
  if(audio_numbers_between_video_ > 1)
  {
    //如果音频帧数大于1，就用上一个音频来校正音频时间戳
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
  bool fine = (delta > - kMaxVideoDeltaTime && delta < kMaxVideoDeltaTime);
  if(!fine)
  {
    delta = kDefaultVideoDeltaTime;
  }
  audio_original_timestamp_ = time;
  audio_corrected_timestamp_ += video_corrected_timestamp_ + delta;
  if(audio_corrected_timestamp_ < 0)
  {
    audio_corrected_timestamp_ = 0;
  }
  return audio_corrected_timestamp_;
}
 
uint32_t TimeCorrector::CorrectVideoTimeStampByVideo(const PacketPtr &packet)  {

  audio_numbers_between_video_ = 0;//新的视频帧开始，两个视频帧之间的音频帧数清0
  int64_t time = packet->TimeStamp();
  if(video_original_timestamp_ == -1)
  {
    video_original_timestamp_ = time;
    video_corrected_timestamp_ = time;
    if(audio_original_timestamp_ != -1)
    {
      //如果音频已经初始化，而视频的时间戳首次出现时与音频的时间戳相差太大，
      //就会调整视频时间戳，使其与音频时间戳对齐
      int32_t delta = audio_original_timestamp_ - video_original_timestamp_;
      if(delta <= -kMaxAudioDeltaTime || delta >= kMaxAudioDeltaTime)
      {
        video_original_timestamp_ = audio_original_timestamp_;
        video_corrected_timestamp_ = audio_corrected_timestamp_;
      }
    }
  }
  int64_t delta = time - video_original_timestamp_;
  bool fine = (delta > - kMaxVideoDeltaTime && delta < kMaxVideoDeltaTime);
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
 
uint32_t TimeCorrector::CorrectAudioTimeStampByAudio(const PacketPtr &packet)  {
  int64_t time = packet->TimeStamp();
  if(audio_original_timestamp_ == -1)
  {
    audio_original_timestamp_ = time;
    audio_corrected_timestamp_ = time;
    return time;
  }
  int64_t delta = time - audio_original_timestamp_;
  bool fine = (delta > - kMaxAudioDeltaTime && delta < kMaxAudioDeltaTime);
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
