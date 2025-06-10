#pragma once
#include <cstdint>
#include <cstddef>

//DtlsHandler定义Dtls通知调用方的事件
namespace tmms
{
    namespace mm
    {
        class Dtls;
        class DtlsHandler
        {
        public:
            virtual void OnDtlsSend(const char* data, size_t size, Dtls* dtls){}
            virtual void OnDtlsHandshakeDone(Dtls* dtls){}//握手完成通知调用方
        };
    }
}
