#pragma once
#include "PlayerUser.h"
#include "User.h"
#include "mmedia/base/Packet.h"
#include <vector>
namespace tmms 
{
  namespace live
  {
    class FlvPlayerUser : public PlayerUser
    {
    public:
      using PlayerUser::PlayerUser;
      
      bool PostFrames();
      UserType GetUserType() const;
    private:
      using User::SetUserType;

      bool PushFrame(PacketPtr &packet, bool is_header);
      bool PushFrames(std::vector<PacketPtr> &list);

      void PushFlvHttpHeader();
      bool http_header_sent_{false};
    };
  }
}
