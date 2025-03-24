#pragma once
#include "AMFAny.h"
namespace tmms
{
  namespace mm
  {
    class AMFNumber : public AMFAny
    {
    public:
      AMFNumber(const std::string &name);
      AMFNumber();
      ~AMFNumber();

      int Decode(const char *data, int size, bool has = false) override;
      bool IsNumber() override;
      double Number() override;
      void Dump() const override;
    private:
      double number_{0.0f};
    };
  }
} 
