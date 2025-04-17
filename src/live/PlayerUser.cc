#include "PlayerUser.h"

using namespace tmms::live;
PlayerUser::PlayerUser(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s)
:User(ptr,stream,s)
{

}
PacketPtr PlayerUser::Meta() const
{
    return meta_;
}
PacketPtr PlayerUser::VideoHeader() const
{
    return video_header_;
}
PacketPtr PlayerUser::AudioHeader() const
{
    return audio_header_;
}
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
