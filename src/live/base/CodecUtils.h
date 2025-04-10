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
      //判断是否是否是序列头
      static bool IsCodecHeader(const PacketPtr &packet);
    };
  }
}
