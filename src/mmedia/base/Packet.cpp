#include "Packet.h"

using namespace tmms::mm;

PacketPtr Packet::NewPacket(int32_t size)
{
    auto block_size = size + sizeof(Packet);
    Packet * packet = (Packet*)new char[block_size];
    memset((void*)packet,0x00,block_size);
    packet->index_ = -1;
    packet->type_ = kPacketTypeUnknowed;
    packet->capacity_ = size;
    packet->ext_.reset();

    return PacketPtr(packet,[](Packet *p){
        delete [](char*)p; 
    });
}