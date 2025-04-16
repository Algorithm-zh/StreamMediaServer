#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpServer.h"
#include "mmedia/rtmp/RtmpHandShake.h"
#include <iostream>

using namespace tmms::network;
using namespace tmms::mm;

EventLoopThread eventloop_thread;
std::thread th;
using RtmpHandShakePtr = std::shared_ptr<RtmpHandShake>;
const char *http_response="HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc,const char ** agrv)
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if(loop)
    {
        InetAddress listen("192.168.1.200:1935");
        TcpServer server(loop,listen);
        server.SetMessageCallback([](const TcpConnectionPtr&con,MsgBuffer &buf){
            RtmpHandShakePtr shake = con->GetContext<RtmpHandShake>(kNormalContext);
            shake->HandShake(buf);
        });
        server.SetNewConnectionCallback([&loop](const TcpConnectionPtr &con){
                RtmpHandShakePtr shake = std::make_shared<RtmpHandShake>(con,false);
                con->SetContext(kNormalContext,shake);
                shake->Start();
                con->SetWriteCompleteCallback([&loop](const TcpConnectionPtr&con){
                std::cout << "write complete host:" << con->PeerAddr().ToIpPort() << std::endl;
                RtmpHandShakePtr shake = con->GetContext<RtmpHandShake>(kNormalContext);
                shake->WriteComplete();
            });
        });
        server.Start();
        while(1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }    
    return 0;
}