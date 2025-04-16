#include "network/base/InetAddress.h"
#include "network/base/SocketOpt.h"
#include <iostream>

using namespace tmms::network;

void TestClient()
{
    int sock = SocketOpt::CreateNonblockingTcpSocket(AF_INET);
    if(sock<0)
    {
        std::cerr << "socket failed.sock:" << sock << " errno:" << errno << std::endl;
        return ;
    }
    InetAddress server("192.168.1.200:34444");
    SocketOpt opt(sock);
    opt.SetNonBlocking(false);
    auto ret = opt.Connect(server);

    std::cout << "Connect ret:" << ret << " errno:" << errno << std::endl
            << " local:"<< opt.GetLocalAddr()->ToIpPort()<< std::endl
            << " peer:" << opt.GetPeerAddr()->ToIpPort()<< std::endl
            << std::endl;
}
void TestServer()
{
    int sock = SocketOpt::CreateNonblockingTcpSocket(AF_INET);
    if(sock<0)
    {
        std::cerr << "socket failed.sock:" << sock << " errno:" << errno << std::endl;
        return ;
    }
    InetAddress server("0.0.0.0:34444");
    SocketOpt opt(sock);
    opt.SetNonBlocking(false);
    opt.BindAddress(server);
    opt.Listen();
    InetAddress addr;
    auto ns = opt.Accept(&addr);

    std::cout << "Accept ret:" << ns << " errno:" << errno << std::endl
            << " addr:"<< addr.ToIpPort()<< std::endl
            << std::endl;
}
int main(int argc,const char ** agrv)
{
    TestServer();
    return 0;
}