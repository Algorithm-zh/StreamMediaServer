#pragma once
#include <cstdint>
namespace tmms
{
  namespace mm
  {
    class StreamWriter
    {
    public:
        StreamWriter(){}
        virtual ~StreamWriter(){}
        virtual void AppendTimeStamp(int64_t pts) = 0;
        virtual int32_t Write(void *buf, uint32_t size) = 0;
        virtual char* Data() = 0;
        virtual int Size() = 0;
    };
  }
}

