#pragma once
#include "../network/net/TcpConnection.h"
#include <string>
#include <memory>
#include <functional>
namespace tmms
{
  namespace network
  {
    class TestContext
    {
      using TestMessageCallback = std::function<void(const TcpConnectionPtr &con, const std::string &message)>;
      enum
      {
        kTestContextHeader = 1,
        kTestContextBody = 2,
      };
    public:
      TestContext(const TcpConnectionPtr &con);
      ~TestContext() = default;
      int ParseMessage(MsgBuffer &buf);
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
