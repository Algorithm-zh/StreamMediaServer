#include "CodecUtils.h"

using namespace tmms::live;

bool CodecUtils::IsCodecHeader(const PacketPtr &packet)
{
    if(packet->PacketSize()>1)
    {
        const char *b = packet->Data() + 1;
        if(*b == 0)
        {
            return true;
        }
    }
    return false;
}

bool CodecUtils::IsKeyFrame(const PacketPtr &packet)
{
    if(packet->PacketSize()>0)
    {
        const char *b = packet->Data();
        return ((*b>>4)&0x0f) == 1;
    }
    return false;
}