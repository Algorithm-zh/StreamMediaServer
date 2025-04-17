#include "RtmpPlayerUser.h"
#include "base/TTime.h"
#include "live/Stream.h"
#include "live/base/LiveLog.h"
#include "mmedia/base/Packet.h"
#include "mmedia/rtmp/RtmpContext.h"
#include "network/net/Connection.h"
#include <memory>
#include <string>
#include <sys/socket.h>
using namespace tmms::live;
using namespace tmms::base;
 
bool RtmpPlayerUser::PostFrames()  {
  
  if(!stream_->Ready() || !stream_->HasMedia())
  {
    return false;
  }
  stream_->GetFrames(std::dynamic_pointer_cast<PlayerUser>(shared_from_this()));
  //按顺序进行发送
  if(meta_)
  {
    auto ret = PushFrame(meta_, true);
    if(ret)
    {
      LIVE_INFO << "rtmp sent meta now:" << base::TTime::NowMs() << "host:" << user_id_;
      meta_.reset();
    }
  }
  else if(audio_header_)
  {
    auto ret = PushFrame(audio_header_, false);
    if(ret)
    {
      LIVE_INFO << "rtmp sent audio header now:" << base::TTime::NowMs() << "host:" << user_id_;
      audio_header_.reset();
    }
  }
  else if(video_header_)
  {
    auto ret = PushFrame(video_header_, false);
    if(ret)
    {
      LIVE_INFO << "rtmp sent video header now:" << base::TTime::NowMs() << "host:" << user_id_;
      video_header_.reset();
    }
  }
  else if(!out_frames_.empty())
  {
    auto ret = PushFrame(out_frames_);
    if(ret)
    {
      out_frames_.clear();
    }
  }
  else
  {
    Deactive();
  }
  return true;
}
 
UserType RtmpPlayerUser::GetUserType() const {
	return UserType::kUserTypePlayerRtmp;
}
 
bool RtmpPlayerUser::PushFrame(PacketPtr &packet, bool is_header)  {
  auto cx = connection_->GetContext<RtmpContext>(kRtmpContext);
  if(!cx || cx->Ready())
  {
    return false;
  }
  int64_t ts = 0;
  if(!is_header)
  {
    ts = time_corrector_.CorrectTimestamp(packet);
  }
  cx->BuildChunk(packet, ts, is_header);
  cx->Send();
  return true;
}
 
bool RtmpPlayerUser::PushFrame(std::vector<PacketPtr> &list)  {

  auto cx = connection_->GetContext<RtmpContext>(kRtmpContext);
  if(!cx || cx->Ready())
  {
    return false;
  }
  int64_t ts = 0;
  for(int i = 0; i < list.size(); i++)
  {
    PacketPtr &packet = list[i];
    ts = time_corrector_.CorrectTimestamp(packet);
    cx->BuildChunk(packet, ts);
  }
  cx->Send();
  return true;
}

