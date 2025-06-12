#include "AACDecoder.h"
#include "mmedia/base/AVTypes.h"
#include "mmedia/base/MMediaLog.h"
#include "neaacdec.h"

using namespace tmms::mm;
 
bool AACDecoder::Init(const std::string &config)  {
  handle_ = NeAACDecOpen();
  unsigned long samplerate = 0;
  unsigned char channels = 0;
  auto ret = NeAACDecInit2(handle_, (unsigned char*)config.c_str(), config.size(), &samplerate, &channels);
  if(ret >= 0)
  {
    WEBRTC_DEBUG << "AACDecoder::Ini ok, samplerate:" << samplerate << " channels:" << channels;
  }
  else
  {
    WEBRTC_ERROR << "AACDecoder::Ini failed. error:" << ret;
    return false;
  }
  return true;
}
 
SampleBuf AACDecoder::Decode(unsigned char *aac, size_t aac_size)  {

  NeAACDecFrameInfo frame_info;
  //return pcm
  char *data = (char*)NeAACDecDecode(handle_, &frame_info, aac, aac_size);
  if(data && frame_info.samples > 0 && frame_info.error == 0)
  {
    int32_t bytes = frame_info.samples * frame_info.channels;
    return SampleBuf(data, bytes);
  }
  else if(frame_info.error > 0)
  {
    WEBRTC_ERROR << "AACDecoder::Decode failed. error:" << frame_info.error;
  }
  return SampleBuf(nullptr, 0);
}
