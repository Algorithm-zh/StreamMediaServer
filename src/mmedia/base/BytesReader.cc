#include "BytesReader.h"
#include <cstdint>
#include <netinet/in.h>
#include <cstring>
using namespace tmms::mm;
uint64_t BytesReader::ReadUint64T(const char *data)
{
  uint64_t in = *((uint64_t*)data);
  uint64_t res = __bswap_64(in);
  double value;
  memcpy(&value, &res, sizeof(double));
  return value;
}
uint32_t BytesReader::ReadUint32T(const char *data)
{
  uint32_t *c = (uint32_t *) data;
  return ntohl(*c);
}
uint32_t BytesReader::ReadUint24T(const char *data)
{
  unsigned char *c = (unsigned char *) data;
  uint32_t val;
  val = (c[0] << 16) | (c[1] << 8) | c[2];
  return val;
}
uint16_t BytesReader::ReadUint16T(const char *data)
{
  uint16_t *c = (uint16_t *) data;
  return ntohs(*c);
}
uint8_t BytesReader::ReadUint8T(const char *data)
{
  return data[0];
}
