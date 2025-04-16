#pragma once
#include "network/net/TcpConnection.h"
#include <string>
#include <functional>
#include <memory>

namespace tmms
{
    namespace network
    {
        using TestMessageCallback = std::function<void(const TcpConnectionPtr&,const std::string &)>;
        class TestContext
        {
            enum
            {
                kTestContextHeader = 1,
                kTestContextBody = 2,
            };
        public:
            TestContext(const TcpConnectionPtr &con);
            ~TestContext() = default;

            int ParseMessage(MsgBuffer & buf);
            void SetTestMessageCallback(const TestMessageCallback &cb);
            void SetTestMessageCallback(TestMessageCallback &&cb);
        private:
            TcpConnectionPtr connection_;
            int state_{kTestContextHeader};
            int32_t message_length_{0};
            std::string message_body_;
            TestMessageCallback cb_;
        };
    }
}