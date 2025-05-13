#pragma once
#include <cstdint>
#include <string>
namespace tmms
{
  namespace mm
  {
    const int32_t kSectionMaxSize = 1020;
    class StreamWriter
    {
    public:
        StreamWriter(){}
        virtual ~StreamWriter(){}
        virtual void AppendTimeStamp(int64_t pts) = 0;
        virtual int32_t Write(void *buf, uint32_t size) = 0;
        virtual char* Data() = 0;
        virtual int Size() = 0;
        void SetSPS(const std::string &sps) 
        {
            sps_ = sps;
        }
        void SetPPS(const std::string &pps)
        {
            pps_ = pps;
        }
        const std::string &GetSPS() const
        {
            return sps_;
        }
        const std::string &GetPPS() const
        {
            return pps_;
        }
        void SetSpsPpsAppended(bool b)
        {
            sps_pps_appended_ = b;
        }
        bool GetSpsPpsAppended() const
        {
            return sps_pps_appended_;
        }
 
    protected:
        std::string sps_;
        std::string pps_;
        bool sps_pps_appended_{false};
    };
  }
}

