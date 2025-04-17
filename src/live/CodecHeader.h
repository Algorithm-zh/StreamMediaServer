#pragma once
#include "mmedia/base/Packet.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace tmms
{
  namespace live
  {
    using namespace tmms::mm;
    class CodecHeader
    {
    public:
      CodecHeader();
      ~CodecHeader();
      PacketPtr Meta(int idx);
      PacketPtr VideoHeader(int idx);//查找第一个比Packet小的header
      PacketPtr AudioHeader(int idx);
      void SaveMeta(const PacketPtr &packet);
      void ParseMeta(const PacketPtr &packet);
      void SaveAudioHeader(const PacketPtr &packet);
      void SaveVideoHeader(const PacketPtr &packet);
      bool ParseCodecHeader(const PacketPtr &packet);
    private:
      PacketPtr video_header_;
      PacketPtr audio_header_;
      PacketPtr meta_;
      int meta_version_{0};
      int audio_version_{0};
      int video_version_{0};
      std::vector<PacketPtr> video_header_packets_;
      std::vector<PacketPtr> audio_header_packets_;
      std::vector<PacketPtr> meta_packets_;
      int64_t start_timestamp_{0};
    };
  }
}
