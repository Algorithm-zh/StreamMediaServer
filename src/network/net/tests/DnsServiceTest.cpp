#include "../../DnsService.h"
#include "../../../base/Singleton.h"
#include <iostream>
#include <vector>
using namespace tmms::network;
int main(int argc, const char **argv) {

  std::vector<InetAddressPtr> list;
  sDnsService->AddHost("www.baidu.com");
  sDnsService->Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  list = sDnsService->GetHostAddress("www.baidu.com");
  for(auto &l : list)
  {
    std::cout << "ip: " << l->ToIpPort() << std::endl;
  }
  while(true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  return 0;
}
