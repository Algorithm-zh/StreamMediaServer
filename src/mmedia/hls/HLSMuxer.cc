#include "HLSMuxer.h"
#include <iostream>
#include "mmedia/base/AVTypes.h"
#include "base/StringUtils.h"
using namespace tmms::mm;
 
HLSMuxer::HLSMuxer(const std::string &session_name)
:stream_name_(session_name)
{
  //获取流名
  auto list = base::StringUtils::SplitString(session_name, "/"); 
  if(list.size() == 3)
  {
    stream_name_ = list[2];
  }
}
 
std::string HLSMuxer::PlayList()  {
  return fragment_window_.GetPlayList();
}

 
bool HLSMuxer::IsCodecHeader(const PacketPtr &packet)  {
  if(packet->PacketSize() > 1)
  {
    //第二位是0则是序列头
    const char *b = packet->Data() + 1;
    if(*b == 0)
    {
      return true;
    }
  }
  return false;
}

void HLSMuxer::OnPacket(PacketPtr &packet)  {
  //往writer中写数据，如果是key帧就把之前累积写入的切片放到窗口里
  if(current_fragment_) 
  {
    auto is_key = packet->IsKeyFrame();
    
    if ((is_key && current_fragment_->Duration() >= min_fragment_size_)
        || current_fragment_->Duration() > max_fragment_size_) 
    {
      fragment_window_.AppendFragment(std::move(current_fragment_));
      current_fragment_.reset();
    }
  }
  //如果没有切片就创建一个切片并开始写数据
  if(!current_fragment_)
  {
    current_fragment_ = fragment_window_.GetIdleFragment();
    if(!current_fragment_)
    {
      current_fragment_ = std::make_shared<Fragment>();
    }
    current_fragment_->Reset();
    current_fragment_->SetBaseFileName(stream_name_);
    current_fragment_->SetSequenceNo(fragment_seq_no_ ++ );
    //切片的开始需要写入patpmt
    encoder_.WritePatPmt(current_fragment_.get());
  }
  if(IsCodecHeader(packet))
  {
    ParseCodec(current_fragment_, packet);
  }
  int64_t dts = packet->TimeStamp();
  encoder_.Encode(current_fragment_.get(), packet, dts);  
}
 
FragmentPtr HLSMuxer::GetFragment(const std::string &name)  {
  return fragment_window_.GetFragmentByName(name);
}
 
void HLSMuxer::ParseCodec(FragmentPtr &fragment, PacketPtr &packet){

  char *data = packet->Data();
  if(packet->IsAudio())
  {
    AudioCodecID id = (AudioCodecID)((*data&0xf0) >> 4);
    encoder_.SetStreamType(fragment.get(), kVideoCodecIDReserved, id);
  }
  else if(packet->IsVideo())
  {
    VideoCodecID id = (VideoCodecID)(*data&0x0f);
    encoder_.SetStreamType(fragment.get(), id, kAudioCodecIDReserved);
  }
}
