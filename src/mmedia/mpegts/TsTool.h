#pragma once
#include "mmedia/base/Packet.h"
#include <cstdint>
#include <string>
namespace tmms
{
  namespace mm
  {
    class TsTool
    {
    public:
      static std::string HexString(uint32_t s);
      static uint32_t CRC32(const void* buf, int size);
      static uint32_t CRC32Ieee(const void* buf, int size);
      static void WritePts(uint8_t *q, int fourbits, int64_t pts);
      static bool IsCodecHeader(const PacketPtr &packet);

    };
  }
}
