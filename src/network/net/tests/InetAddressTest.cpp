#include "network/base/InetAddress.h"
#include <string>
#include <iostream>

using namespace tmms::network;

int main(int argc,const char ** agrv)
{
    std::string host;
    while(std::cin >> host)
    {
        InetAddress addr(host);
        std::cout << "host:" << host << std::endl
                << " ip:" << addr.IP() << std::endl
                << " port:" << addr.Port() << std::endl
                << " lan:" << addr.IsLanIp() << std::endl
                << " wan:" << addr.IsWanIp() << std::endl
                << " loop:" << addr.IsLoopbackIp() << std::endl;
    }
    return 0;
}