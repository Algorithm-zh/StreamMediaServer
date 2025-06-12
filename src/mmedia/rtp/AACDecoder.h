#pragma once

#include "faad.h"
#include "neaacdec.h"
#include "mmedia/base/AVTypes.h"
#include <cstdint>
#include <string>


namespace tmms
{
  namespace mm
  {
    class AACDecoder
    {
    public:
      AACDecoder() = default;
      ~AACDecoder() = default;
      bool Init(const std::string &config);
      SampleBuf Decode(unsigned char *aac, size_t aac_size);
    private:
      NeAACDecHandle handle_{nullptr};
    };
  }
}
