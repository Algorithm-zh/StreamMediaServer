#include "WebrtcPlayerUser.h"
#include "base/Config.h"
#include "live/Session.h"
#include "live/base/LiveLog.h"
#include <cstdlib>
#include <random>
using namespace tmms::live;


 
WebrtcPlayerUser::WebrtcPlayerUser(const ConnectionPtr &ptr, const StreamPtr &stream, const SessionPtr &s)
:PlayerUser(ptr, stream, s) 
{
  local_ufrag_ = GetUFrag(8);
  local_passwd_ = GetUFrag(32);
  uint32_t audio_ssrc = GetSsrc(10);
  uint32_t video_ssrc = audio_ssrc + 1;

  sdp_.SetLocalUFrag(local_ufrag_);
  sdp_.SetLocalPasswd(local_passwd_);
  sdp_.SetAudioSsrc(audio_ssrc);
  sdp_.SetVideoSsrc(video_ssrc);
  dtls_cert_.Init();
  sdp_.SetFingerprint(dtls_cert_.Fingerprint());

  auto config = sConfigMgr->GetConfig();
  if(config)
  {
    auto serverinfo = config->GetServiceInfo("webrtc", "udp");
    if(serverinfo)
    {
      sdp_.SetServerAddr(serverinfo->addr);
      sdp_.SetServerPort(serverinfo->port);
    }
  }
  sdp_.SetStreamName(s->SessionName());
}
 
bool WebrtcPlayerUser::PostFrames()  {
	return false;
}
 
UserType WebrtcPlayerUser::GetUserType() const {
	return UserType::kUserTypePlayerWebRTC;
}
 
bool WebrtcPlayerUser::ProcessOfferSdp(const std::string &sdp)  {
  return sdp_.Decode(sdp);
}
 
const std::string &WebrtcPlayerUser::LocalUFrag() const {
  return sdp_.GetLocalUFrag();
}
 
const std::string &WebrtcPlayerUser::LocalPasswd() const {
  return sdp_.GetLocalPasswd();
}
 
const std::string &WebrtcPlayerUser::RemoteUFrag() const {
  return sdp_.GetRemoteUFrag();
}
 
std::string WebrtcPlayerUser::GetUFrag(int size)  {
  static std::string table = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::string frag;
  frag.resize(size);
  static std::mt19937 mt{std::random_device{}()};
  static std::uniform_int_distribution<> rand(0, table.size());

  for(int i = 0; i < size; ++i)
  {
    frag[i] = table[rand(mt) % table.size()];
  }

  return frag;
}
 
uint32_t WebrtcPlayerUser::GetSsrc(int size)  {
  static std::mt19937 mt{std::random_device{}()};
  static std::uniform_int_distribution<> rand(10000000,99999999);

  return rand(mt);
}
 
std::string WebrtcPlayerUser::BuildAnswerSdp()  {
	return sdp_.Encode();
}
 
void WebrtcPlayerUser::SetConnection(const ConnectionPtr &conn)  {
  User::SetConnection(conn); 
}
