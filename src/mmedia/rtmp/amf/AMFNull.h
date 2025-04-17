#pragma once
#include "AMFAny.h"

namespace tmms
{
    namespace mm
    {
        class AMFNull:public AMFAny
        {
        public:
            AMFNull(const std::string &name);
            AMFNull();
            ~AMFNull();

            int Decode(const char *data, int size,bool has=false) override;
            bool IsNull() override;
            void Dump() const override;
        private:
            bool b_{false};
        };
    }
}
