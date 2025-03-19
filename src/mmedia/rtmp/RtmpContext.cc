#include "RtmpContext.h"
#include "../base/MMediaLog.h"
#include "RtmpHandShake.h"
using namespace tmms::mm;
 
RtmpContext::RtmpContext(const TcpConnectionPtr &conn, RtmpHandler *handler, bool client)
:handshake_(conn, client), connection_(conn), handler_(handler)  
{
 
}
 
int32_t RtmpContext::Parse(MsgBuffer &buf)  {
  if(state_ == kRtmpHandShake)
  {
    auto ret = handshake_.HandShake(buf);
    //握手成功
    if(ret == 0)
    {
      state_ = kRtmpMessage;
      if(buf.ReadableBytes() > 0)
      {
        return Parse(buf);
      }
    }
    else if(ret == -1)
    {
      RTMP_ERROR << "rtmp handshake failed\n";
    }
    else if(ret == 2)
    {
      state_ = kRtmpWaitingDone;
    }
  }
  else if(state_ == kRtmpMessage)
  {
    
  }
}
 
void RtmpContext::OnWriteComplete()  {
 
  if(state_ == kRtmpHandShake)
  {
    handshake_.WriteComplete();
  }
  else if(state_ == kRtmpWaitingDone)
  {
    state_ = kRtmpMessage;
  }
  else if(state_ == kRtmpMessage)
  {
    
  }
}
 
void RtmpContext::StartHandShake()  {
 
  handshake_.Start();
}
 
