#include "CodecUtils.h"
using namespace tmms::live;
 

bool CodecUtils::IsCodecHeader(const PacketPtr &packet)  {
  if(packet->PacketSize() > 1)
  {
    const char *b = packet->Data() + 1;
    if(*b == 0)
    {
      return true;
    }
  }
  return false;
}
