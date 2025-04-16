#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/UdpClient.h"

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
        InetAddress server("192.168.1.200:34444");
        std::shared_ptr<UdpClient> client = std::make_shared<UdpClient>(loop,server);
        client->SetRecvMsgCallback([](const InetAddress &addr,MsgBuffer &buf){
            std::cout << "host:" << addr.ToIpPort() << " msg:" << buf.Peek() << std::endl;
            buf.RetrieveAll();
        });
        client->SetCloseCallback([](const UdpSocketPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        client->SetWriteCompleteCallback([](const UdpSocketPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " write complete. " << std::endl;
            }
        });
        client->SetConnectedCallback([&client](const UdpSocketPtr&con,bool connected){
            if(connected)
            {
                client->Send("11111",strlen("11111"));
            }
        });
        client->Connect();
        while(1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }    
    return 0;
}