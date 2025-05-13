#include "AudioDemux.h"
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/NalBitStream.h"
using namespace tmms::mm;
 
int32_t AudioDemux::OnDemux(const char* data, size_t size, std::list<SampleBuf> &list)  {
  if(size < 1)
  {
    DEMUX_ERROR << "param error.size < 1";  
    return -1;
  }
  sound_format_ = (AudioCodecID)((*data >> 4)&0x0f);
  sound_rate_ = (SoundRate)((*data&0xc0) >> 2);
  sound_size_ = (SoundSize)((*data&0x02) >> 1);
  sound_type_ = (SoundChannel)((*data&0x01));
  //DEMUX_DEBUG << "sound format:" << sound_format_ << ", rate:" << sound_rate_
  //            << ", size:" << sound_size_ << ", type:" << sound_type_;
  if(sound_format_ == kAudioCodecIDMP3)
  {
    return DemuxMP3(data, size, list);
  }
  else if(sound_format_ == kAudioCodecIDAAC)
  {
    return DemuxAAC(data, size, list);
  }
  else
  {
    DEMUX_ERROR << "no support sound format:" << sound_format_;
  }
  return -1;
}
 
int32_t AudioDemux::DemuxAAC(const char* data, size_t size, std::list<SampleBuf> &list)  {

  //aac第二个字节为AACPacketType
  AACPacketType type = (AACPacketType)data[1];
  if(type == kAACPacketTypeAACSequenceHeader)
  {
    if(size - 2 > 0)
    {
      return DemuxAACSequenceHeader(data + 2, size - 2);
    }
  }
  else if(type == kAACPacketTypeAACRaw)
  {
    if(!aac_ok_)//如果还没收到序列头就退出
    {
      return -1;
    }
    list.emplace_back(SampleBuf(data + 2, size - 2));
  }
  return 0;
}
 
int32_t AudioDemux::DemuxMP3(const char* data, size_t size, std::list<SampleBuf> &list)  {

  //去掉第一个字节，第二个字节开始就是mp3的raw数据
  list.emplace_back(SampleBuf(data + 1, size - 1));
  return 0;
}
 
int32_t AudioDemux::DemuxAACSequenceHeader(const char* data, int size)  {

  if(size < 2)
  {
    DEMUX_ERROR << "demux aac seq header failed. size < 2";
    return -1;
  }
  NalBitStream stream(data, size);

  aac_object_ = (AACObjectType)stream.GetWord(4);
  aac_sample_rate_ = stream.GetWord(5);
  aac_channel_ = stream.GetWord(4);

  aac_ok_ = true;
  return 0;
}
