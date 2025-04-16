#include "network/DnsService.h"

#include <iostream>
using namespace tmms::network;
int main(int argc,const char ** agrv)
{
    std::vector<InetAddressPtr> list;
    DnsService::GetHostInfo("www.baidu.com",list);
    for(auto &i:list)
    {
        std::cout << "ip:" << i->IP() << std::endl;
    }
    return 0;
}