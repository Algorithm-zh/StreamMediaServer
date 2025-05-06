#include "PSIWriter.h"
#include "mmedia/base/BytesWriter.h"
#include "TsTool.h"
#include <cstdint>
#include <cstring>
using namespace tmms::mm;
 
void PSIWriter::SetVersion(uint8_t v)  {
  version_ = v;
}
 
void PSIWriter::PushSection(StreamWriter *w, uint8_t *buf, int len)  {
  //输出ts  
  uint8_t packet[188], *q;
  uint8_t *p = buf;
  bool first = false;

  while(len > 0)
  {
    q = packet;
    first = (p == buf);
    //sync_byte固定字节
    *q ++ = 0x47;
    //pid按顺序存储transport_error_indicator1 payload_unit_start_indicator1 transport_priority1 PID13
    auto b = pid_ >> 8;
    if(first)
    {
      //payload_unit_start_indicator设置为1
      b |= 0x40;//0100 0000
    }
    *q ++ = b;//高八位
    *q ++ = pid_;//把低8位也写进去
    cc_ = (cc_ + 1) & 0xf;
    //transport_scrambling_control2 adaptation_field_control2 continuity_counter4
    *q ++ = 0x10 | cc_;//0001
    if(first)
    {
      *q ++ = 0;
    }
    auto len1 = 188 - (q - packet);
    if(len1 > len)
    {
      len1 = len;
    }
    memcpy(q, p, len1);
    p += len1;
    auto left = 188 - (q - packet);
    if(left > 0)
    {
      memset(q, 0xff, left);
    }
    w->Write(packet, 188);
    p += len1;
    len -= len1;
  }
}
 
int PSIWriter::WriteSection(StreamWriter *w, int id, int sec_num, int last_sec_num, uint8_t *buf, int len)  {

  //组装PSI 
  uint8_t section[1024], *p;
  auto total_len = len + 3 + 5 + 4;
  p = section;
  *p ++ = table_id_;
  //section_syntax_indicator1 reserved_future_use1 reserved2 section_length12
  BytesWriter::WriteUint16T((char*)p, (5 + 4 + len) | 0xb000);
  p += 2;
  //tsid16
  BytesWriter::WriteUint16T((char*)p, id);
  //reserved2 version_number5 current_next_indicator1
  *p ++ = 0xc1 | (version_ << 1);//1100 0001
  *p ++ = sec_num;
  *p ++ = last_sec_num;
  //section data
  memcpy(p, buf, len);
  p += len;
  //计算crc
  auto crc = TsTool::CRC32(section, total_len - 4);
  BytesWriter::WriteUint32T((char*)p, crc);
  PushSection(w, section, total_len);
  return 0;
}
