#pragma once
#include "user/PlayerUser.h"
#include "live/Session.h"
#include "live/base/TimeCorrector.h"
#include "base/GopMgr.h"
#include "base/CodecHeader.h"
#include "mmedia/hls/HLSMuxer.h"
#include "mmedia/base/Packet.h"
#include <mutex>
#include <string>
#include <memory>
#include <atomic>
#include <vector>

namespace tmms
{
  namespace live
  {
    using namespace tmms::mm;
    using PlayerUserPtr = std::shared_ptr<PlayerUser>;
    class Session;
    class Stream
    {
    public:
      Stream(Session &s, const std::string &session_name);
      //时间处理成员函数
      int64_t ReadyTime() const;
      int64_t SinceStart() const;
      bool Timeout();
      int64_t DataTime() const;
      //流信息成员函数
      const std::string &SessionName() const;
      int32_t StreamVersion() const;
      bool HasMedia() const;//media info
      bool Ready() const;
      //输入成员函数(important)
      void AddPacket(PacketPtr &&packet);
      //输出成员函数
      void GetFrames(const PlayerUserPtr &user);
      
      bool HasVideo() const;
      bool HasAudio() const;
      //hls
      std::string PlayList()
      {
        return muxer_.PlayList();
      }
      FragmentPtr GetFragment(const std::string &name)
      {
        return muxer_.GetFragment(name);
      }

    private:
      void ProcessHls(PacketPtr &packet);
      void SetReady(bool ready);

      bool LocateGop(const PlayerUserPtr &user);//查找到所有的codecheader并且定位到gop才能发音视频数据
      void SkipFrame(const PlayerUserPtr &user);
      void GetNextFrame(const PlayerUserPtr &user);//输出具体的数据帧

      int64_t data_coming_time_{0};//音视频数据第一次来的时间
      int64_t start_timestamp_{0};//创建stream的时间
      int64_t ready_time_{0};//收到关键帧的时间
      std::atomic_int64_t stream_time_{0};//有数据来的时间，用于判断流有无超时 推流的和检测超时的线程都在使用
  
      Session &session_;
      std::string session_name_;
      std::atomic_int64_t  frame_index_{-1};//当前帧的索引
      uint32_t packet_buffer_size_{1000};//1000frame
      std::vector<PacketPtr> packet_buffer_;
      bool has_meta_{false};
      bool has_video_{false}; 
      bool has_audio_{false};
      bool ready_{false};
      std::atomic_int32_t stream_version_{-1};
      //input use
      GopMgr gop_mgr_;
      CodecHeader codec_headers_;
      TimeCorrector time_corrector_;
      std::mutex lock_;
      
      HLSMuxer muxer_;
      
    };
  }
}
