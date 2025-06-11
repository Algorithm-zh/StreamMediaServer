#pragma once
#include "PlayerUser.h"
#include "mmedia/base/Packet.h"
#include "mmedia/webrtc/Sdp.h"
#include "mmedia/webrtc/Dtls.h"
#include "mmedia/webrtc/Srtp.h"
#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>

namespace tmms
{
  namespace live
  {
    using namespace tmms::mm;
    class WebrtcPlayerUser : public PlayerUser, public DtlsHandler
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
      std::string BuildAnswerSdp();//生成回复客户端的sdp
      void SetConnection(const ConnectionPtr &conn) override;
      void OnDtlsRecv(const char *buf, size_t size);

    private:
      void OnDtlsSend(const char* data, size_t size, Dtls* dtls) override;
      void OnDtlsHandshakeDone(Dtls* dtls) override;//握手完成通知调用方
      static std::string GetUFrag(int size);
      static uint32_t GetSsrc(int size);

      std::string local_ufrag_;
      std::string local_passwd_;
      Sdp sdp_;
      Dtls dtls_;
      PacketPtr packet_;
      struct sockaddr_in6 addr_;
      socklen_t addr_len_{sizeof(struct sockaddr_in6)};
      bool dtls_done_{false};
      Srtp srtp_;
    };
  }
}
