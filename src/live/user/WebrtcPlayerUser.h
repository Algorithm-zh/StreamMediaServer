#pragma once
#include "PlayerUser.h"
#include <cstdint>

namespace tmms
{
  namespace live
  {
    class WebrtcPlayerUser : public PlayerUser
    {
    public:
      explicit WebrtcPlayerUser(const ConnectionPtr &ptr, const StreamPtr &stream, const SessionPtr &s);
      bool PostFrames() override;
      UserType GetUserType() const override;

    private:
    };
  }
}
