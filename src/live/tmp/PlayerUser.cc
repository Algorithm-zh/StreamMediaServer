#include "PlayerUser.h"
using namespace tmms::live;
//CodecHeader成员函数
PacketPtr PlayerUser::Meta() const
{
  return meta_;
}
PacketPtr PlayerUser::AudioHeader() const
{
  return audio_header_;
}
PacketPtr PlayerUser::VideoHeader() const
{
  return video_header_;
}
//发送完之后清空数据
void PlayerUser::ClearMeta()
{
  meta_.reset();
}
void PlayerUser::ClearAudioHeader()
{
  audio_header_.reset();
}
void PlayerUser::ClearVideoHeader()
{
  video_header_.reset();
}
TimeCorrector& PlayerUser::GetTimeCorrector()
{
  return time_corrector_;
}
