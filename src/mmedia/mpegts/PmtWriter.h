#pragma once
#include "PSIWriter.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace tmms
{
  namespace mm
  {
    //原始流信息
    struct ProgramInfo
    {
        uint8_t stream_type;//8bits
        int16_t elementary_pid;//13bits
    };
    using ProgramInfoPtr = std::shared_ptr<ProgramInfo>;
    class PmtWriter : public PSIWriter
    {
    public:
      PmtWriter();
      ~PmtWriter() = default;
      void WritePmt(StreamWriter *w);
      void AddProgramInfo(ProgramInfoPtr &program);
      void SetPcrPid(int32_t pid);

    private:
      uint16_t pcr_pid_{0xe000};
      std::vector<ProgramInfoPtr> programs_;
    };
  }
}
