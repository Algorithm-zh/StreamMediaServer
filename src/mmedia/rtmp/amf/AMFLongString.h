#pragma once
#include "AMFAny.h"

namespace tmms
{
    namespace mm
    {
        class AMFLongString:public AMFAny
        {
        public:
            AMFLongString(const std::string &name);
            AMFLongString();
            ~AMFLongString();

            int Decode(const char *data, int size,bool has=false) override;
            bool IsString() override;
            const std::string& String() override;
            void Dump() const override;
        private:
            std::string string_;
        };
    }
}