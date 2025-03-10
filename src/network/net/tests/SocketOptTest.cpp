#include "../../base/SocketOpt.h"
#include "../../base/InetAddress.h"
#include <cerrno>
#include <iostream>
using namespace tmms::network;

void TestCLient()
{

  int sock = SocketOpt::CreateNonblockingTcpSocket(AF_INET);
  if(sock < 0)
  {
    std::cerr << "socket failed.sock" << sock << "errno:" << errno << std::endl;
    return;
  }
  InetAddress server("172.18.12.3:34444");
  std::cout << "server:" << server.ToIpPort() << std::endl;
  SocketOpt opt(sock);
  opt.SetNonBlocking(false);
  auto ret = opt.Connect(server);

  std::cout << "connect ret:" << ret << "errno:" << errno << std::endl
            << "local:" << opt.GetLocalAddr()->ToIpPort() << std::endl 
            << "peer:" << opt.GetPeerAddr()->ToIpPort() << std::endl;
}

void TestServer()
{

  int sock = SocketOpt::CreateNonblockingTcpSocket(AF_INET);
  if(sock < 0)
  {
    std::cerr << "socket failed.sock" << sock << "errno:" << errno << std::endl;
    return;
  }
  InetAddress server("0.0.0.0:34444");
  std::cout << "server:" << server.ToIpPort() << std::endl;
  SocketOpt opt(sock);
  opt.SetNonBlocking(false);
  opt.BindAddress(server);
  opt.Listen();
  InetAddress addr;
  auto ret = opt.Accept(&addr);

  std::cout << "accept ret:" << ret << "errno:" << errno << std::endl
            << "addr:" << addr.ToIpPort() << std::endl;
}

int main(int argc, char *argv[]) {

  TestServer();
  return 0;
}



