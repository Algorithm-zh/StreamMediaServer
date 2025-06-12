#include "Rtp.h"
#include "mmedia/base/MMediaLog.h"
#include <netinet/in.h>
#include <cstring>

using namespace tmms::mm;
 
Rtp::Rtp(int32_t pt)
:version_(2),
padding_(0),
extern_(0),
csrc_count_(0),
marker_(0),
payload_type_(pt),
sequence_(0),
timestamp_(0),
ssrc_(0),
sample_(0)
{
 
}
 
void Rtp::EncodeHeader(char *buf)  {
 
  char *header = buf;
  header[0] = (char)(version_ << 6 | padding_ << 5 | extern_ << 4 | csrc_count_);
  header[1] = (char)(marker_ << 7 | payload_type_);
  header[2] = (char)(sequence_ >> 8);
  header[3] = (char)(sequence_ & 0xff);
  header += 4;
  
  uint32_t t = htonl(timestamp_);
  std::memcpy(header, &t, sizeof(uint32_t));
  header += 4;

  uint32_t ss = htonl(ssrc_);
  std::memcpy(header, &ss, sizeof(uint32_t));
  header += 4;
}
 
int32_t Rtp::HeaderSize() const {
  int32_t header_size = 12;

  if(csrc_count_ > 0)
  {
    //one csrc is 4 bytes
    header_size += 4 * csrc_count_;
  }
  return header_size;
}
 
void Rtp::SetMarker(bool on)  {
  marker_ = on ? 1 : 0;
}
 
void Rtp::SetSequenceNumber(uint16_t s)  {
  sequence_ = s;
}
 
uint16_t Rtp::SequenceNumber() const {
  return sequence_;
}
 
void Rtp::SetTimestamp(uint32_t timestamp)  {
  timestamp_ = timestamp; 
}
 
void Rtp::SetSsrc(uint32_t ssrc)  {
  ssrc_ = ssrc;
}
 
void Rtp::SetSample(int32_t s)  {
  sample_ = s;
}
