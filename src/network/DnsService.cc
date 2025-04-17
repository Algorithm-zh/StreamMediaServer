#include "DnsService.h"
#include <functional>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cstring>

using namespace tmms::network;

namespace
{
    static InetAddressPtr inet_address_null; 
}
DnsService::~DnsService()
{

}

void DnsService::AddHost(const std::string& host)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = hosts_info_.find(host);
    if(iter!=hosts_info_.end())
    {
        return;
    }
    hosts_info_[host] = std::vector<InetAddressPtr>();
}
InetAddressPtr DnsService::GetHostAddress(const std::string &host,int index)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = hosts_info_.find(host);
    if(iter!=hosts_info_.end())
    {
        auto list = iter->second;
        if(list.size()>0)
        {
            return list[index%list.size()];
        }
    }    
    return inet_address_null;
}
std::vector<InetAddressPtr> DnsService::GetHostAddress(const std::string &host)
{
    std::lock_guard<std::mutex> lk(lock_);
    auto iter = hosts_info_.find(host);
    if(iter!=hosts_info_.end())
    {
        auto list = iter->second;
        return list;
    } 
    return std::vector<InetAddressPtr>();
}
void DnsService::UpdateHost(const std::string &host,std::vector<InetAddressPtr> &list)
{
    std::lock_guard<std::mutex> lk(lock_);
    hosts_info_[host].swap(list);
}
std::unordered_map<std::string,std::vector<InetAddressPtr>> DnsService::GetHosts()
{
    std::lock_guard<std::mutex> lk(lock_);
    return hosts_info_;
}
void DnsService::SetDnsServiceParam(int32_t interval,int32_t sleep,int32_t retry)
{   
    interval_ = interval;
    sleep_ = sleep;
    retry_ = retry;
}
void DnsService::Start()
{
    running_ = true;
    thread_ = std::thread(std::bind(&DnsService::OnWork,this));
}
void DnsService::Stop()
{
    running_ = false;
    if(thread_.joinable())
    {
        thread_.join();
    }
}
void DnsService::GetHostInfo(const std::string&host,std::vector<InetAddressPtr>&list)
{
    struct addrinfo ainfo,*res;
    memset(&ainfo,0x00,sizeof(ainfo));
    ainfo.ai_family = AF_UNSPEC;
    ainfo.ai_flags = AI_PASSIVE;
    ainfo.ai_socktype = SOCK_DGRAM;
    auto ret = ::getaddrinfo(host.c_str(),nullptr,&ainfo,&res);
    if(ret == -1 || res == nullptr)
    {
        return;
    }
    struct addrinfo *rp = res;
    for(;rp!=nullptr;rp=rp->ai_next)
    {
        InetAddressPtr peeraddr = std::make_shared<InetAddress>();
        if(rp->ai_family == AF_INET)
        {
            char ip[16] = {0,};
            struct sockaddr_in *saddr = (struct sockaddr_in*)rp->ai_addr;
            ::inet_ntop(AF_INET,&(saddr->sin_addr.s_addr),ip,sizeof(ip));
            peeraddr->SetAddr(ip);
            peeraddr->SetPort(ntohs(saddr->sin_port));
        }
        else if(rp->ai_family == AF_INET6)
        {
            char ip[INET6_ADDRSTRLEN] = {0,};
            struct sockaddr_in6 *saddr = (struct sockaddr_in6*)rp->ai_addr;
            ::inet_ntop(AF_INET6,&(saddr->sin6_addr),ip,sizeof(ip));
            peeraddr->SetAddr(ip);
            peeraddr->SetPort(ntohs(saddr->sin6_port));
            peeraddr->SetIsIPV6(true);
        }
        list.push_back(peeraddr);
    }
}
void DnsService::OnWork()
{
    while(running_)
    {
        auto host_infos = GetHosts();
        for(auto &host:host_infos)
        {
            for(int i=0;i<retry_;i++)
            {
                std::vector<InetAddressPtr> list;
                GetHostInfo(host.first,list);
                if(list.size()>0)
                {
                    UpdateHost(host.first,list);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
    }
}