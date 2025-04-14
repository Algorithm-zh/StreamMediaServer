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
      //这里用的是 flv 的 tag 结构, 它规定第一个字节的高四位就是帧类型，低四位就是编码器 id
      static bool IsKeyFrame(const PacketPtr &packet);
    };
  }
}
