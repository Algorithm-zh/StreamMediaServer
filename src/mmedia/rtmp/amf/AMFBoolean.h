#pragma once
#include "AMFAny.h"
namespace tmms
{
  namespace mm
  {
    class AMFBoolean : public AMFAny
    {
    public:
      AMFBoolean(const std::string &name);
      AMFBoolean();
      ~AMFBoolean();

      int Decode(const char *data, int size, bool has = false) override;
      bool IsBoolean() override;
      bool Boolean() override;
      void Dump() const override;
    private:
      bool b_{false};
    };
  }
} 
