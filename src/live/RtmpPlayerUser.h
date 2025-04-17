#pragma once

#include "PlayerUser.h"

namespace tmms
{
    namespace live
    {
        class RtmpPlayerUser:public PlayerUser
        {
        public:
            explicit RtmpPlayerUser(const ConnectionPtr &ptr,const StreamPtr &stream,const SessionPtr &s);

            bool PostFrames();
            UserType GetUserType() const;
        private:
            using User::SetUserType;

            bool PushFrame(PacketPtr &packet,bool is_header);
            bool PushFrames(std::vector<PacketPtr> &list);
        };
    }
}