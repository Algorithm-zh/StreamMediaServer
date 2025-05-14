#pragma once
#include <cstdint>
#include "StreamWriter.h"
namespace tmms
{
  namespace mm
  {
    class PSIWriter
    {
    public:
      PSIWriter() = default;
      virtual ~PSIWriter() = default;
      void SetVersion(uint8_t v);
      //把section数据填充到PSI里，不同的表 id不一样, len为psi表正文(section data)长度
      int WriteSection(StreamWriter *w, int id, int sec_num, int last_sec_num, uint8_t *buf, int len);

    protected:
      //把section输出到ts包的缓存里
      void PushSection(StreamWriter *w, uint8_t *buf, int len);

      int8_t cc_{-1};//用于检测数据包丢失
      uint16_t pid_{0xe000};
      uint8_t table_id_{0x00};
      int8_t version_{0x00};

    };
  }
}
