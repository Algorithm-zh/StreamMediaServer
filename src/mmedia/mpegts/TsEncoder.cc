#include "TsEncoder.h"
#include "mmedia/base/AVTypes.h"
using namespace tmms::mm;

 
int32_t TsEncoder::Encode(StreamWriter *writer, PacketPtr &data, int64_t dts)  {

  if(data->IsAudio())
  {
    audio_encoder_.EncodeAudio(writer, data, dts);
  }
  else if(data->IsVideo())
  {
    bool key = data->IsKeyFrame();
    video_encoder_.EncodeVideo(writer, key, data, dts);
  }
  return 0;
}
 
void TsEncoder::SetStreamType(StreamWriter *w, VideoCodecID vc, AudioCodecID ac)  {
 
  TsStreamType atype = kTsStreamReserved;
  TsStreamType vtype = kTsStreamReserved;
  if(ac == kAudioCodecIDAAC)
  {
    atype = kTsStreamAudioAAC;
    audio_pid_ = 0x101;
  }
  else if(ac == kAudioCodecIDMP3)
  {
    atype = kTsStreamAudioMp3;
    audio_pid_ = 0x102;
  }

  //video
  if(vc == kVideoCodecIDAVC)
  {
    vtype = kTsStreamVideoH264;
    video_pid_ = 0x100;
  }

  bool writer = false;
  //说明变了，则需要写入pat pmt
  if(atype != kTsStreamReserved && atype != audio_type_)
  {
    auto p = std::shared_ptr<ProgramInfo>();
    p->stream_type = atype;
    p->elementary_pid = audio_pid_;
    pmt_writer_.AddProgramInfo(p);
    writer = true;
    audio_encoder_.SetPid(audio_pid_);
    audio_encoder_.SetStreamType(atype);
    audio_type_ = atype;
  }
  if(vtype != kTsStreamReserved && vtype != video_type_)
  {
    auto p = std::shared_ptr<ProgramInfo>();
    p->stream_type = vtype;
    p->elementary_pid = video_pid_;
    pmt_writer_.AddProgramInfo(p);
    writer = true;
    video_encoder_.SetPid(video_pid_);
    video_encoder_.SetStreamType(vtype);
    pmt_writer_.SetPcrPid(video_pid_);
    video_type_ = vtype;
  }
  if(writer)
  {
    WritePatPmt(w);
  }
}
//如果音视频的头收到了 ,则写入pat pmt
int32_t TsEncoder::WritePatPmt(StreamWriter *w)  {

  if(audio_pid_ != 0xe000 && video_pid_ != 0xe000)
  {
    pat_writer_.WritePat(w);
    pmt_writer_.WritePmt(w);
  }
  return 0;
}
