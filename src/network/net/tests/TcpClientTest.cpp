#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/TcpClient.h"

#include <iostream>

using namespace tmms::network;
EventLoopThread eventloop_thread;
std::thread th;
const char *http_request="GET / HTTP/1.0\r\nHost: 192.168.1.200\r\nAccept: */*\r\nContent-Type: text/plain\r\nContent-Length: 0\r\n\r\n";
const char *http_response="HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc,const char ** agrv)
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if(loop)
    {
        InetAddress server("192.168.1.200:34444");
        std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(loop,server);
        client->SetRecvMsgCallback([](const TcpConnectionPtr&con,MsgBuffer &buf){
            std::cout << "host:" << con->PeerAddr().ToIpPort() << " msg:" << buf.Peek() << std::endl;
            buf.RetrieveAll();
        });
        client->SetCloseCallback([](const TcpConnectionPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " closed." << std::endl;
            }
        });
        client->SetWriteCompleteCallback([](const TcpConnectionPtr &con){
            if(con)
            {
                std::cout << "host:" << con->PeerAddr().ToIpPort() << " write complete. " << std::endl;
            }
        });
        client->SetConnectCallback([](const TcpConnectionPtr&con,bool connected){
            if(connected)
            {
                auto size = htonl(strlen(http_request));
                con->Send((const char*)&size,sizeof(size));
                con->Send(http_request,strlen(http_request));
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