#include "Stream.h"
#include "base/TTime.h"
#include "live/base/CodecUtils.h"
#include "live/base/LiveLog.h"
#include "mmedia/base/Packet.h"
#include <mutex>
#include <utility>
using namespace tmms::live;
using namespace tmms::base;
 
Stream::Stream(const std::string &session_name)
: session_name_(session_name), packet_buffer_(packet_buffer_size_)
{
  stream_time_ = TTime::NowMs(); 
  start_timestamp_ = TTime::NowMs();
}
 
int64_t Stream::ReadyTime() const {
  return ready_time_;
}
 
int64_t Stream::SinceStart() const {
  return TTime::NowMs() - start_timestamp_;
}
 
bool Stream::Timeout()  {
  auto delta = TTime::NowMs() - stream_time_;
  if(delta > 20 * 1000)//20s
  {
    return true;
  }
  return false;
}
 
int64_t Stream::DataTime() const {
  return data_coming_time_;
}
 
const std::string &Stream::SessionName() const {
  return session_name_;
}
 
int32_t Stream::StreamVersion() const {
  return stream_version_;
}
 
bool Stream::HasMeta() const {
  return has_meta_ || has_video_ || has_audio_;
}
 
void Stream::SetReady(bool ready)  {
  ready_ = true; 
  ready_time_ = TTime::NowMs();
}
 
bool Stream::Ready() const {
	return ready_;
}
 

void Stream::AddPacket(PacketPtr &&packet)  {
  
  auto t = time_corrector_.CorrectTimestap(packet);
  packet->SetTimeStamp(t);
  std::lock_guard<std::mutex> lock(lock_);
  auto index = ++ frame_index_;
  packet->SetIndex(index);
  //如果是视频并且是关键帧，则设置关键帧标记
  if(packet->IsVideo() && CodecUtils::IsKeyFrame(packet))
  {
    SetReady(true);
    packet->SetPacketType(kPacketTypeVideo | kFrameTypeKeyFrame);
  }
  if(CodecUtils::IsCodecHeader(packet))
  {
    codec_headers_.ParseCodecHeader(packet);
    if(packet->IsVideo())
    {
      has_video_ = true;
    }
    else if(packet->IsAudio())
    {
      has_audio_ = true;
    }
    else if(packet->IsMeta())
    {
      has_meta_ = true;
    }
    stream_version_ ++;
  }
  gop_mgr_.AddFrame(packet);
  packet_buffer_[index % packet_buffer_size_] = std::move(packet);
  auto min_idx = frame_index_ - packet_buffer_size_;
  if(min_idx > 0)
  {
    //删除已经过期的gop
    gop_mgr_.ClearExpriedGop(min_idx);
  }
  if(data_coming_time_ == 0)
  {
    data_coming_time_ = TTime::NowMs();
  }
  stream_time_ = TTime::NowMs();

}
 
void Stream::GetFrames(const PlayerUserPtr &user)  {
 
  if(!HasMeta())
  {
    return;
  }
  //如果还有值，说明序列头还没有发送完毕
  if(user->meta_ || user->audio_header_ || user->video_header_ || !user->out_frames_.empty())
  {
    return ;
  }
  std::lock_guard<std::mutex> lock(lock_);
  //已经开始发送序列头
  if(user->out_index_ >= 0)
  {
    //需不需要跳帧
    int min_idx = frame_index_ - packet_buffer_size_;
    int content_latency = user->GetAppInfo()->content_latency;
    if(user->out_index_ < min_idx 
      || (gop_mgr_.LatestTimestamp() - user->out_frame_timestamp_) > 2 * content_latency)
    {
      LIVE_INFO << "need skip out index:" << user->out_index_ 
                << " min_idx:" << min_idx
                << ", out timestamp:" << user->out_frame_timestamp_
                << ", latest timestamp:" << gop_mgr_.LatestTimestamp();
      SkipFrame(user);
    }
  }
  else
  {
    //还没有发送序列头，那就查找序列头
    if(LocateGop(user))
    {
      return ;   
    }
  }
  //输出完就取数据
  GetNextFrame(user);
}
 
bool Stream::LocateGop(const PlayerUserPtr &user)  {
  int content_latency = user->GetAppInfo()->content_latency;
  int latency = 0;
  auto idx = gop_mgr_.GetGopByLatency(content_latency, latency);
  if(idx != -1)
  {
    //因为getNextFrame取的是out_index_ + 1,所以这里要减去1
    user->out_index_ = idx - 1;   
  }
  else
  {
    //等待关键帧超时
    auto elapsed = user->ElapsedTime();
    if(elapsed >= 1000 && !user->wait_timeout_)
    {
      LIVE_DEBUG << "wait gop keyframe timeout.host:" << user->user_id_; 
      user->wait_timeout_ = true;
    }
    return false;
  }

  //看要不要发meta
  user->wait_meta_ = user->wait_meta_ && has_meta_;
  if(user->wait_meta_)
  {
    auto meta = codec_headers_.Meta(idx);
    if(meta)
    {
      user->wait_meta_ = false;
      user->meta_ = meta;//输出完会被清空
      user->meta_index_ = meta->Index();
    }
  }
  user->wait_audio_ = user->wait_audio_ && has_audio_;
  if(user->wait_audio_)
  {
    auto audio = codec_headers_.AudioHeader(idx);
    if(audio)
    {
      user->wait_audio_ = false;
      user->audio_header_ = audio;
      user->audio_header_index_ = audio->Index();
    }
  }
  user->wait_video_ = user->wait_video_ && has_video_;
  if(user->wait_video_)
  {
    auto video = codec_headers_.VideoHeader(idx);
    if(video)
    {
      user->wait_video_ = false;
      user->video_header_ = video;
      user->video_header_index_ = video->Index();
    }
  }
  //idx为-1说明没有定位到gop
  if(user->wait_meta_ || user->wait_audio_ || user->wait_video_ || idx == -1)
  {
    auto elapsed = user->ElapsedTime(); 
    if(elapsed >= 1000 && !user->wait_timeout_)
    {
      LIVE_DEBUG << "wait gop keyframe timeout elapsed:" << elapsed 
              << "ms, frame index: " << frame_index_
              << ", gop size:" << gop_mgr_.GopSize()
              << ", host:" << user->user_id_;
      user->wait_timeout_ = true;
    }
    return false;
  }
  //重置状态位,保证下一次能继续输出序列头
  user->wait_meta_ = true;
  user->wait_audio_ = true;
  user->wait_video_ = true;
  user->out_version_ = stream_version_;

  auto elapsed = user->ElapsedTime();
  LIVE_DEBUG << "locate gop success.elapsed:" << elapsed 
            << "ms, gop idx" << idx
            << ", frame index: " << frame_index_
            << ", latency:" << latency 
            << ", user:" << user->user_id_;
  return true;
}
 
void Stream::SkipFrame(const PlayerUserPtr &user)  {
 
  int content_latency = user->GetAppInfo()->content_latency;
  int latency = 0;
  auto idx = gop_mgr_.GetGopByLatency(content_latency, latency);
  if(idx == -1 || idx < user->out_index_)
  {
    return ;
  }
  auto meta = codec_headers_.Meta(idx);
  if(meta)
  {
    //说明这个比正在输出的更新,那就输出新的
    if(meta->Index() > user->meta_index_)
    {
      user->meta_ = meta;
      user->meta_index_ = meta->Index();
    }
  }
  auto audio = codec_headers_.AudioHeader(idx);
  if(audio)
  {
    if(audio->Index() > user->audio_header_index_)
    {
      user->audio_header_ = audio;
      user->audio_header_index_ = audio->Index();
    }
  }
  auto video = codec_headers_.VideoHeader(idx);
  if(video)
  {
    if(video->Index() > user->video_header_index_)
    {
      user->video_header_ = video;
      user->video_header_index_ = video->Index();
    }
  }
  LIVE_DEBUG << "skip frame " << user->out_index_ << "->" << idx
            << ", latency:" << latency 
            << ", user:" << user->user_id_
            << ", frame_index:" << frame_index_;
  user->out_index_ = idx - 1;
}
 
void Stream::GetNextFrame(const PlayerUserPtr &user)  {
 
  auto idx = user->out_index_ + 1;
  auto max_idx = frame_index_.load();
  //一次输出10个包
  for(int i = 0; i < 10; i++)
  {
    if(idx > max_idx) 
    {
      break;
    }
    auto &pkt = packet_buffer_[idx % packet_buffer_size_];
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
