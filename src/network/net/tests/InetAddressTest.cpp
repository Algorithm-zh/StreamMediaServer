#include "../../base/InetAddress.h"
#include <iostream>
using namespace tmms::network;

int main(int argc, char *argv[]) {
  std::string host;
  while(std::cin >> host) {
    InetAddress addr(host);
    std::cout << "host:" << host << std::endl 
              << " ip:" << addr.IP() << std::endl
              << " port:" << addr.Port() << std::endl
              << " ipv6:" << addr.IsIpV6() << std::endl
              << " wan:" << addr.IsWanIp() << std::endl
              << " lan:" << addr.IsLanIp() << std::endl
              << " loopback:" << addr.IsLoopbackIp() << std::endl;
  }
  return 0;
}
