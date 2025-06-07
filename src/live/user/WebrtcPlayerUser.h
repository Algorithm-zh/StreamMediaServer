#pragma once
#include "PlayerUser.h"
#include "mmedia/webrtc/Sdp.h"
#include "mmedia/webrtc/DtlsCerts.h"
#include <cstdint>

namespace tmms
{
  namespace live
  {
    using namespace tmms::mm;
    class WebrtcPlayerUser : public PlayerUser
    {
    public:
      explicit WebrtcPlayerUser(const ConnectionPtr &ptr, const StreamPtr &stream, const SessionPtr &s);
      bool PostFrames() override;
      UserType GetUserType() const override;
      //sdp成员函数
      bool ProcessOfferSdp(const std::string &sdp);
      const std::string &LocalUFrag() const;
      const std::string &LocalPasswd() const;
      const std::string &RemoteUFrag() const;
      static std::string GetUFrag(int size);
      static uint32_t GetSsrc(int size);
      std::string BuildAnswerSdp();//生成回复客户端的sdp

    private:
      std::string local_ufrag_;
      std::string local_passwd_;
      Sdp sdp_;
      DtlsCerts dtls_cert_;
    };
  }
}
