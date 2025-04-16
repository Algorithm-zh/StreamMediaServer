#include "network/net/Acceptor.h"
#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include <iostream>

using namespace tmms::network;
EventLoopThread eventloop_thread;
std::thread th;

const char *http_response="HTTP/1.0 200 OK\r\nServer: tmms\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n";
int main(int argc,const char ** agrv)
{
    eventloop_thread.Run();
    EventLoop *loop = eventloop_thread.Loop();

    if(loop)
    {   std::vector<TcpConnectionPtr> list;
        InetAddress server("192.168.1.200:34444");
        std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop,server);
        acceptor->SetAcceptCallback([&loop,&server,&list](int fd,const InetAddress &addr){
            std::cout << "host:" << addr.ToIpPort() << std::endl;
            TcpConnectionPtr connection = std::make_shared<TcpConnection>(loop,fd,server,addr);
            connection->SetRecvMsgCallback([](const TcpConnectionPtr&con,MsgBuffer &buf){
                std::cout << "recv msg:" << buf.Peek() << std::endl;
                buf.RetrieveAll();
                con->Send(http_response,strlen(http_response));
            });
            connection->SetWriteCompleteCallback([&loop](const TcpConnectionPtr&con){
                std::cout << "write complete host:" << con->PeerAddr().ToIpPort() << std::endl;
                loop->DelEvent(con);
                con->ForceClose();
            });
            list.push_back(connection);
            loop->AddEvent(connection);
        });
        acceptor->Start();
        while(1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }    
    return 0;
}