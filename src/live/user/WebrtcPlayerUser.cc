#include "WebrtcPlayerUser.h"
#include "live/base/LiveLog.h"
using namespace tmms::live;


 
WebrtcPlayerUser::WebrtcPlayerUser(const ConnectionPtr &ptr, const StreamPtr &stream, const SessionPtr &s)
:PlayerUser(ptr, stream, s) 
{
 
}
 
bool WebrtcPlayerUser::PostFrames()  {
	return false;
}
 
UserType WebrtcPlayerUser::GetUserType() const {
	return UserType::kUserTypePlayerWebRTC;
}
