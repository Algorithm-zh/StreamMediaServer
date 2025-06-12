#include "TOpusEncoder.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/Packet.h"
#include <cstring>
#include <opus/opus.h>
#include <opus/opus_defines.h>
#include <opus/opus_types.h>

using namespace tmms::mm;
 
TOpusEncoder::~TOpusEncoder()  {
 
  if(encoder_)
  {
    opus_encoder_destroy(encoder_);
    encoder_ = nullptr;
  }
}
 
bool TOpusEncoder::Init(int sample, int channels)  {

  int error = 0;
  encoder_ = opus_encoder_create(sample, channels, OPUS_APPLICATION_AUDIO, &error);
  if(!encoder_)
  {
    WEBRTC_ERROR << "opus_encoder_create failed. error:" << error;
  }
  //set resistant loss packet
  opus_encoder_ctl(encoder_, OPUS_SET_PACKET_LOSS_PERC(20));
  //让编码器额外插入一些纠错信息，以便在解码端恢复丢失的音频帧
  opus_encoder_ctl(encoder_, OPUS_SET_INBAND_FEC(1));

  frame_bytes_ = frame_size_ * channels * sizeof(opus_uint16);
  return true;
}
 
bool TOpusEncoder::Encode(SampleBuf &pcm, std::list<PacketPtr> &pkts)  {
  memcpy(pcm_buf_ + pcm_bytes_, pcm.addr, pcm.size);
  pcm_bytes_ += pcm.size;

  //less one frame size
  if(pcm_bytes_ < frame_bytes_)
  {
    return false;
  }
  unsigned char out_buffer[kOpusMaxSize];
  while(pcm_bytes_ >= frame_bytes_)
  {
    auto bytes = opus_encode(encoder_, (const opus_int16*)pcm_buf_, frame_size_, out_buffer, kOpusMaxSize);
    pcm_bytes_ -= frame_bytes_;
    memmove(pcm_buf_, pcm_buf_ + frame_bytes_, pcm_bytes_);//往前移动数据
    
    if(bytes > 0)
    {
      PacketPtr packet = Packet::NewPacket(bytes);
      memcpy(packet->Data(), out_buffer, bytes);
      packet->SetPacketSize(bytes);
      packet->SetPacketType(kPacketTypeAudio);
      pkts.emplace_back(packet);
    }
  }
  return !pkts.empty();
}
