#pragma once
#include <string>
#include <memory>
#include <cstring>
#include <cstdint>

namespace tmms
{
  namespace mm
  {
    enum
    {
      kPacketTypeVideo = 1,
      kPacketTypeAudio = 2,
      kPacketTypeMeta = 4,
      kPacketTypeMeta3 = 8, 
      kFrameTypeKeyFrame = 16,
      kFrameTypeIDR = 32,
      kPacketTypeUnKnowed = 255,
    };
    class Packet;
    using PacketPtr = std::shared_ptr<Packet>;
//为了节省空间，改变对齐方式,用一个字节对齐
#pragma pack(push)
#pragma pack(1)
    class Packet
    {
    public:
      Packet(int32_t size)
      :capacity_(size)
      {

      }
      ~Packet(){}
      //内存申请成员函数
      static PacketPtr NewPacket(int32_t size);

      //包类型判断成员函数
      bool IsVideo() const { return (type_ & kPacketTypeVideo) == kPacketTypeVideo; }
      bool IsKeyFrame() const
      {
        return (type_ & kPacketTypeVideo) == kPacketTypeVideo
                && (type_ & kFrameTypeKeyFrame) == kFrameTypeKeyFrame;
      }
      bool IsAudio() const { return type_ == kPacketTypeAudio; }
      bool IsMeta() const { return  type_ == kPacketTypeMeta; }
      bool IsMeta3() const { return  type_ == kPacketTypeMeta3; }

      //包大小成员函数
      inline int32_t PacketSize() const { return size_; }
      //还有多少空间 
      inline int Space() const{ return capacity_ - size_; }     
      inline void SetPacketSize(size_t len) { size_ = len; }
      //在原来的基础上 + len
      inline void UpdatePacketSize(size_t len) { size_ += len; }

      //设置额外数据成员函数
      template <typename T> inline std::shared_ptr<T> Ext() const
      {
        return std::static_pointer_cast<T>(ext_);
      }
      inline void SetExt(const std::shared_ptr<void> &ext) { ext_ = ext; }

      //其它成员函数
      void SetIndex(int32_t index) { index_ = index; }
      int32_t Index() const { return index_; }
      void SetPacketType(int32_t type) { type_ = type; }
      int32_t PacketType() const { return type_; }
      void SetTimeStamp(uint64_t timestamp) { timestamp_ = timestamp; }
      uint64_t TimeStamp() const { return timestamp_; }
      //返回整个packet数据开始的位置
      inline char *Data() { return (char *)this + sizeof(Packet); }
    private:
      int32_t type_{kPacketTypeUnKnowed};
      uint32_t size_{0};
      int32_t index_{-1};
      uint64_t timestamp_{0};
      uint32_t capacity_{0};
      std::shared_ptr<void> ext_;
    };
#pragma pack()
  }
}
