#pragma once
#include <cstdint>
#include <mutex>
#include "StreamWriter.h"
namespace tmms
{
  namespace mm
  {
    class TestStreamWriter : public StreamWriter
    {
    public:
        TestStreamWriter();
        ~TestStreamWriter();
        void AppendTimeStamp(int64_t pts) override{};
        int32_t Write(void *buf, size_t size) override;
        char* Data() override{return nullptr;};
        int32_t Size() const override{return 0;};
    private:
      int fd_{-1};
    };
  }
}

