#pragma once
#include "mmedia/base/Packet.h"
#include "mmedia/hls/Fragment.h"
#include "mmedia/hls/FragmentWindow.h"
#include "mmedia/mpegts/TsEncoder.h"
#include <string>
namespace tmms
{
  namespace mm
  {
    using FragmentPtr = std::shared_ptr<class Fragment>;
    class HLSMuxer
    {
    public:
      HLSMuxer(const std::string &session_name);
      ~HLSMuxer() = default;
      std::string PlayList();
      void OnPacket(PacketPtr &packet);
      FragmentPtr GetFragment(const std::string &name);
      void ParseCodec(FragmentPtr &fragment, PacketPtr &packet);//生成patpmt的

    private:
      bool IsCodecHeader(const PacketPtr &packet);

      FragmentWindow fragment_window_;
      TsEncoder encoder_;
      FragmentPtr current_fragment_;
      std::string stream_name_;
      int64_t fragment_seq_no_{0};
      int32_t min_fragment_size_{3000};
      int32_t max_fragment_size_{12000};
    };
  }
}
