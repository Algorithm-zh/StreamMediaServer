#pragma once
#include "PSIWriter.h"
#include <cstdint>

namespace tmms
{
  namespace mm
  {
    class PatWriter : public PSIWriter
    {
    public:
      PatWriter();
      ~PatWriter() = default;
      void WritePat(StreamWriter *w);

    private:
      uint16_t program_number_{0x0001};
      uint16_t pmt_pid_{0x1001};
      uint16_t transport_stream_id_{0x0001};
    };
  }
}
