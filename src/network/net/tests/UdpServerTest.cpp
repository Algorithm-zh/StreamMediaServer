#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/UdpServer.h"

#include <iostream>

using namespace tmms::network;
EventLoopThread eventloop_thread;
std::thread th;

int main(int argc,const char ** agrv)
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if(loop)
    {
        InetAddress listen("192.168.1.200:34444");
        std::shared_ptr<UdpServer> server = std::make_shared<UdpServer>(loop,listen);
        server->SetRecvMsgCallback([&server](const InetAddress &addr,MsgBuffer &buf){
            std::cout << "host:" << addr.ToIpPort() << " msg:" << buf.Peek() << std::endl;
            struct sockaddr_in6 sock_addr;
            addr.GetSockAddr((struct sockaddr*)&sock_addr);
            server->Send(buf.Peek(),buf.ReadableBytes(),(struct sockaddr*)&sock_addr,sizeof(sock_addr));
            buf.RetrieveAll();
        });
        server->SetCloseCallback([](const UdpSocketPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        server->SetWriteCompleteCallback([](const UdpSocketPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " write complete. " << std::endl;
            }
        });

        server->Start();
        while(1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }    
    return 0;
}