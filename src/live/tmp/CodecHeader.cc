#include "CodecHeader.h"
#include "base/TTime.h"
#include "mmedia/base/Packet.h"
#include "live/base/LiveLog.h"
#include "mmedia/rtmp/amf/AMFObject.h"
#include <sstream>
using namespace tmms::live;
using namespace tmms::mm;
 
CodecHeader::CodecHeader()  {

  start_timestamp_ = tmms::base::TTime::NowMs();
}
 
CodecHeader::~CodecHeader()  {
 
}
 
PacketPtr CodecHeader::Meta(int idx)  {
  if(idx <= 0)
  {
    return meta_;
  }
  //从后往前查找最新的播放
  auto iter = meta_packets_.rbegin();
  for(; iter != meta_packets_.rend(); ++iter)
  {
    PacketPtr pkt = *iter;
    if(pkt->Index() <= idx)
    {
      return pkt;
    }
  }
  return meta_;
}
 
PacketPtr CodecHeader::VideoHeader(int idx)  {
  if(idx <= 0)
  {
    return video_header_;
  }
  //从后往前查找最新的播放
  auto iter = video_header_packets_.rbegin();
  for(; iter != video_header_packets_.rend(); ++iter)
  {
    PacketPtr pkt = *iter;
    if(pkt->Index() <= idx)
    {
      return pkt;
    }
  }
  return video_header_;
}
 
PacketPtr CodecHeader::AudioHeader(int idx)  {
  if(idx <= 0)
  {
    return audio_header_;
  }
  //从后往前查找最新的播放
  auto iter = audio_header_packets_.rbegin();
  for(; iter != audio_header_packets_.rend(); ++iter)
  {
    PacketPtr pkt = *iter;
    if(pkt->Index() <= idx)
    {
      return pkt;
    }
  }
  return audio_header_;
}
 
void CodecHeader::SaveMeta(const PacketPtr &packet)  {
 
  meta_ = packet;
  ++ meta_version_;

  meta_packets_.emplace_back(packet);

  LIVE_TRACE << "save meta ,meta version:" << meta_version_
              << ", size:" << packet->PacketSize()
              << " elapse:" << TTime::NowMs() - start_timestamp_ << "ms\n";
}
 
void CodecHeader::ParseMeta(const PacketPtr &packet)  {
 
  AMFObject obj;
  if(!obj.Decode(packet->Data(), packet->PacketSize()))
  {
    return;
  }
  std::stringstream ss;
  ss << "ParseMeta ";

  //视频属性
  AMFAnyPtr widthPtr = obj.Property("width");
  if(widthPtr)
  {
    ss << ", width:" << widthPtr->Number() << " ";
  }
  AMFAnyPtr heightPtr = obj.Property("height");
  if(heightPtr)
  {
    ss << ", height:" << heightPtr->Number() << " ";
  }
  AMFAnyPtr videocodecidPtr = obj.Property("videocodecid");
  if(videocodecidPtr)
  {
    ss << ", videocodecidPtr:" << videocodecidPtr->Number() << " ";
  }
  AMFAnyPtr frameratePtr = obj.Property("framerate");
  if(frameratePtr)
  {
    ss << ", framerate:" << frameratePtr->Number() << " ";
  }
  //码率
  AMFAnyPtr videodataratePtr = obj.Property("videodatarate");
  if(videodataratePtr)
  {
    ss << ", videodatarate:" << videodataratePtr->Number() << " ";
  }

  //音频属性
  AMFAnyPtr audiosampleratePtr = obj.Property("audiosamplerate");
  if(audiosampleratePtr)
  {
    ss << ", audiosamplerate:" << audiosampleratePtr->Number() << " ";
  }
  AMFAnyPtr audiosamplesizePtr = obj.Property("audiosamplesize");
  if(audiosamplesizePtr)
  {
    ss << ", audiosamplesize:" << audiosamplesizePtr->Number() << " ";
  }
  AMFAnyPtr audiocodecidPtr = obj.Property("audiocodecid");
  if(audiocodecidPtr)
  {
    ss << ", audiocodecid:" << audiocodecidPtr->Number() << " ";
  }
  AMFAnyPtr audiodataratePtr = obj.Property("audiodatarate");
  if(audiodataratePtr)
  {
    ss << ", audiodatarate:" << audiodataratePtr->Number() << " ";
  }

  //其它属性
  AMFAnyPtr durationPtr = obj.Property("duration");
  if(durationPtr)
  {
    ss << ", duration:" << durationPtr->Number() << " ";
  }
  AMFAnyPtr encoderPtr = obj.Property("encoder");
  if(encoderPtr)
  {
    ss << ", encoder:" << encoderPtr->String() << " ";
  }
  AMFAnyPtr serverPtr = obj.Property("server");
  if(serverPtr)
  {
    ss << ", server:" << serverPtr->String() << " ";
  }

  LIVE_TRACE << ss.str();
}
 
void CodecHeader::SaveAudioHeader(const PacketPtr &packet)  {
 
  audio_header_ = packet;
  ++ audio_version_;

  audio_header_packets_.emplace_back(packet);

  LIVE_TRACE << "save audio header ,audio version:" << audio_version_
              << ", size:" << packet->PacketSize()
              << " elapse:" << TTime::NowMs() - start_timestamp_ << "ms\n";
}
 
void CodecHeader::SaveVideoHeader(const PacketPtr &packet)  {
 
  video_header_ = packet;
  ++ video_version_;

  video_header_packets_.emplace_back(packet);

  LIVE_TRACE << "save video header ,video version:" << video_version_
              << ", size:" << packet->PacketSize()
              << " elapse:" << TTime::NowMs() - start_timestamp_ << "ms\n";
}
 
bool CodecHeader::ParseCodecHeader(const PacketPtr &packet)  {

  //LIVE_TRACE << "ParseCodecHeader";
  if(packet->IsMeta())
  {
    SaveMeta(packet);
    ParseMeta(packet);
  }
  else if(packet->IsAudio())
  {
    SaveAudioHeader(packet);
  }
  else if(packet->IsVideo())
  {
    SaveVideoHeader(packet);
  }
  return true;
}
