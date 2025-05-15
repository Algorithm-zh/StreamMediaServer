#include "Fragment.h"
#include "mmedia/base/MMediaLog.h"
#include "mmedia/base/Packet.h"
#include "base/TTime.h"
#include <algorithm>

using namespace tmms::mm;
 
void Fragment::AppendTimeStamp(int64_t dts)  {
 
  if(start_dts_ == -1)
  {
    start_dts_ = dts;
  }
  start_dts_ = std::min(start_dts_, dts);
  duration_ = dts - start_dts_;
}
 
int32_t Fragment::Write(void *buf, size_t size)  {
  if(!data_)
  {
    data_ = Packet::NewPacket(buf_size_);
  }
  if(data_->Space() < size)
  {
    buf_size_ += kFragmentStepSize;
    while(data_size_ + size > buf_size_)
    {
      buf_size_ += kFragmentStepSize;
    }
    PacketPtr new_pkt = Packet::NewPacket(buf_size_);
    memcpy(new_pkt->Data(), data_->Data(), data_->PacketSize());
    new_pkt->SetPacketSize(data_->PacketSize());
    data_ = new_pkt;
  }
  memcpy(data_->Data() + data_->PacketSize(), buf, size);
  return 0;
}
 
int32_t Fragment::Size() const {
  return data_->PacketSize();
}
 
char *Fragment::Data()  {
  return data_->Data() + data_->PacketSize();
}
 
int64_t Fragment::Duration() const {
  return duration_;
}
 
const std::string &Fragment::FileName() const {
  return filename_;
}
 
void Fragment::SetBaseFileName(const std::string &v)  {
  filename_.clear();
  filename_.append(v);//流名
  filename_.append("_");
  filename_.append(std::to_string(base::TTime::NowMs()));
  filename_.append(".ts");
}
 
int32_t Fragment::SequenceNo() const {
	return sequence_no_;
}
 
void Fragment::SetSequenceNo(int32_t no)  {
  sequence_no_ = no;
}
 
void Fragment::Reset()  {
  duration_ = 0; 
  sequence_no_ = 0;
  data_size_ = 0;
  start_dts_ = -1;
  if(data_)
  {
    data_->SetPacketSize(0);
  }
}
 
PacketPtr &Fragment::FragmentData()  {
  return data_;
}
