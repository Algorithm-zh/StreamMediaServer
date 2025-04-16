#pragma once

#include "mmedia/base/Packet.h"

namespace tmms
{
    namespace live
    {
        using namespace tmms::mm;
        class CodecUtils
        {
        public:
            static bool IsCodecHeader(const PacketPtr &packet);
            static bool IsKeyFrame(const PacketPtr &packet);
        };
    }
}