#include "Sdp.h"
#include "base/StringUtils.h"
#include "mmedia/base/MMediaLog.h"
#include <cstdlib>
#include <sstream>
using namespace tmms::mm;
 
bool Sdp::Decode(const std::string &sdp)  {
  
  auto list = base::StringUtils::SplitString(sdp, "\n");
  if(list.size() == 0)
  {
    return false;
  }
  static const std::string rtpmap_token = "a=rtpmap:";
  static const std::string ice_ufrag_token = "a=ice-ufrag:";
  static const std::string ice_pwd_token = "a=ice-pwd:";
  static const std::string fingerprint_token = "a=fingerprint:";
  for(auto &line:list)
  {
    if(base::StringUtils::StartsWith(line, ice_ufrag_token))
    {
      remote_ufrag_ = line.substr(ice_ufrag_token.size());
      WEBRTC_DEBUG << "remote ufrag:" << remote_ufrag_;
    }
    else if(base::StringUtils::StartsWith(line, ice_pwd_token))
    {
      remote_passwd_ = line.substr(ice_pwd_token.size());
      WEBRTC_DEBUG << "remote passwd:" << remote_passwd_;
    }
    else if(base::StringUtils::StartsWith(line, fingerprint_token))
    {
      fingerprint_ = line.substr(fingerprint_token.size());
      WEBRTC_DEBUG << "fingerprint:" << fingerprint_;
    }
    else if(base::StringUtils::StartsWith(line, rtpmap_token))
    {
      std::string content = line.substr(rtpmap_token.size()); 
      auto pos = content.find_first_of(" ");
      if(pos == std::string::npos)
      {
        continue;
      }
      //payloadtype
      int32_t pt = std::atoi(content.substr(0, pos).c_str());
      auto pos1 = content.find_first_of("/", pos + 1);
      if(pos1 == std::string::npos)
      {
        continue;
      }
      //encode name
      std::string name = content.substr(pos + 1, pos1 - pos - 1);
      if(audio_payload_type_ == -1 && name == "opus")
      {
        audio_payload_type_ = pt;
        WEBRTC_DEBUG << "audio payload type:" << audio_payload_type_;
      }
      else if(video_payload_type_ == -1 && name == "H264")
      {
        video_payload_type_ = pt;
        WEBRTC_DEBUG << "video payload type:" << video_payload_type_;
      }
    }
    
  }
	return true;
}
 
const std::string &Sdp::GetRemoteUFrag() const {
	return remote_ufrag_;
}
 
const std::string &Sdp::GetFingerprint() const {
	return fingerprint_;
}
 
int32_t Sdp::GetVideoPayloadType() const {
  return video_payload_type_;
}
 
int32_t Sdp::GetAudioPayloadType() const {
	return audio_payload_type_;
}
 
void Sdp::SetFingerprint(const std::string &fp)  {
  fingerprint_ = fp;
}
 
void Sdp::SetStreamName(const std::string &name)  {
  //切分session_name
  auto list = base::StringUtils::SplitString(name, ":");
  if(list.size() == 3)
  {
    stream_name_ = list[2];
  }
  else
  {
    stream_name_ = name;
  }
}
 
void Sdp::SetLocalUFrag(const std::string &frag)  {
  local_ufrag_ = frag;
}
 
void Sdp::SetLocalPasswd(const std::string &pwd)  {
  local_passwd_ = pwd;
}

void Sdp::SetServerPort(uint16_t port)  {
  server_port_ = port;
}
 
//网卡实际地址 
void Sdp::SetServerAddr(const std::string &addr)  {
  server_addr_ = addr;
}
 
void Sdp::SetVideoSsrc(uint32_t ssrc)  {
  video_ssrc_ = ssrc;
}
 
void Sdp::SetAudioSsrc(int32_t ssrc)  {
  audio_ssrc_ = ssrc;
}
 
const std::string &Sdp::GetLocalPasswd() const {
  return local_passwd_;
}

const std::string &Sdp::GetLocalUFrag() const {
   return local_ufrag_;
}
 
uint32_t Sdp::VideoSsrc() const {
  return video_ssrc_;
}
 
uint32_t Sdp::AudioSsrc() const {
  return audio_ssrc_;
}
 
//sdp组装
std::string Sdp::Encode()  {
  std::ostringstream ss;
  ss << "v=0\n";
  ss << "o=tmms 701762458683606775 2 IN IP4 0.0.0.0\n";
  ss << "s=" << stream_name_ << "\n";
  ss << "c=IN IP4 0.0.0.0\n";
  ss << "t=0 0\n";
  //媒体信息描述
  ss << "a=group:BUNDLE 0 1\n";
  ss << "a=msid-semantic: WMS " << stream_name_ << "\n";
  //音频信息描述
  if(video_payload_type_ != -1 && audio_payload_type_ != -1)
  {
    ss << "m=audio 9 UDP/TLS/RTP/SAVPF " << audio_payload_type_ << "\n";
    ss << "a=rtpmap:" << audio_payload_type_ << " opus/48000/2\n";
    ss << "a=fmtp:" << audio_payload_type_ << " minptime=10;stereo=1;useinbandfec=1\n";
    ss << "c=IN IP4 0.0.0.0\n";
    ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
    ss << "a=ice-pwd:" << local_passwd_ << "\n";
    ss << "a=fingerprint:" << fingerprint_ << "\n";
    ss << "a=setup:passive\n";//被动连接
    ss << "a=mid:0\n";//复用一个连接，音频为0,视频为1  
    ss << "a=sendonly\n";
    ss << "a=rtcp-mux\n";
    ss << "a=rtcp-rsize\n";
    ss << "a=rtcp-fb:" << audio_payload_type_ << " transport-cc\n";
    ss << "a=rtcp-fb:" << audio_payload_type_ << " nack\n";
    ss << "a=ssrc:" << audio_ssrc_ << " cname:" << stream_name_ << "\n";
    ss << "a=ssrc:" << audio_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_audio\n";
    ss << "a=ssrc:" << audio_ssrc_ << " mslabel:" << stream_name_ << "\n";
    ss << "a=ssrc:" << audio_ssrc_ << " label:" << stream_name_ << "_audio\n";
    //ice candidate 最高级别的host
    ss << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
  }
  if(video_payload_type_ != -1)
  {
    ss << "m=video 9 UDP/TLS/RTP/SAVPF " << video_payload_type_ << "\n";
    ss << "a=rtpmap:" << video_payload_type_ << " H264/90000\n";
    ss << "c=IN IP4 0.0.0.0\n";
    ss << "a=ice-ufrag:" << local_ufrag_ << "\n";
    ss << "a=ice-pwd:" << local_passwd_ << "\n";
    ss << "a=fingerprint:" << fingerprint_ << "\n";
    ss << "a=setup:passive\n";//被动连接
    ss << "a=mid:1\n";//复用一个连接，音频为0,视频为1
    ss << "a=sendonly\n";
    ss << "a=rtcp-mux\n";
    ss << "a=rtcp-rsize\n";
    ss << "a=rtcp-fb:" << video_payload_type_ << " ccm fir\n";
    ss << "a=rtcp-fb:" << video_payload_type_ << " goog-remb\n";
    ss << "a=rtcp-fb:" << video_payload_type_ << " nack\n";
    ss << "a=rtcp-fb:" << video_payload_type_ << " nack pli\n";
    ss << "a=rtcp-fb:" << video_payload_type_ << " transport-cc\n";
    ss << "a=ssrc:" << video_ssrc_ << " cname:" << stream_name_ << "\n";
    ss << "a=ssrc:" << video_ssrc_ << " msid:" << stream_name_ << " " << stream_name_ << "_video\n";
    ss << "a=ssrc:" << video_ssrc_ << " mslabel:" << stream_name_ << "\n";
    ss << "a=ssrc:" << video_ssrc_ << " label:" << stream_name_ << "_video\n";
    ss << "a=candidate:0 1 udp 2130706431 " << server_addr_ << " " << server_port_ << " typ host generation 0\n";
  }
  return ss.str();
}
 
