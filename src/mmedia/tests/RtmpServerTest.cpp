#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "mmedia/rtmp/RtmpServer.h"
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
        RtmpServer server(loop,listen);

        server.Start();
        while(1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }    
    return 0;
}