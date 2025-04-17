#include "GopMgr.h"
#include "live/base/LiveLog.h"

using namespace tmms::live;

void GopMgr::AddFrame(const PacketPtr &packet)
{
    lastest_timestamp_ = packet->TimeStamp();

    if(packet->IsKeyFrame())
    {
        gops_.emplace_back(packet->Index(),packet->TimeStamp());
        max_gop_length_ = std::max(max_gop_length_,gop_length_);
        total_gop_length_ += gop_length_;
        gop_numbers_ ++;
        gop_length_ = 0;
    }
    gop_length_++;
}
int32_t GopMgr::MaxGopLength() const
{
    return max_gop_length_;
}
size_t GopMgr::GopSize() const
{
    return gops_.size();
}
int GopMgr::GetGopByLatency(int content_latency, int &latency) const
{
    int got = -1;
    latency = 0;
    auto iter = gops_.rbegin();
    for(;iter!=gops_.rend();++iter)
    {
        int item_latency = lastest_timestamp_ - iter->timestamp;
        if(item_latency<=content_latency)
        {
            got = iter->index;
            latency = item_latency;
        }
        else 
        {
            break;
        }
    }
    return got;
}
void GopMgr::ClearExpriedGop(int min_idx)
{
    if(gops_.empty())
    {
        return ;
    }
    for(auto iter = gops_.begin();iter!= gops_.end();)
    {
        if(iter->index <= min_idx)
        {
            iter = gops_.erase(iter);
        }
        else 
        {
            iter++;
        }
    }
}

void GopMgr::PrintAllGop()
{
    std::stringstream ss;

    ss << "all gop:";
    for(auto iter = gops_.begin();iter!= gops_.end();iter++)
    {
        ss << "[" << iter->index <<","<< iter->timestamp<< "]";
    }
    LIVE_TRACE << ss.str() << "\n";
}
