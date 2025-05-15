#include "FragmentWindow.h"
#include "mmedia/base/MMediaLog.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <utility>
using namespace tmms::mm;
 
namespace 
{
  static FragmentPtr fragment_null;
}
FragmentWindow::FragmentWindow(int32_t size)  
:window_size_(size)  {
 
}
 
FragmentWindow::~FragmentWindow()  {
 
}
 
void FragmentWindow::AppendFragment(FragmentPtr &&fragment)  {
  {
    std::lock_guard<std::mutex> lock(lock_);
    fragments_.emplace_back(std::move(fragment));
  }
  Shrink();
  UpdatePlayList();
}
 
FragmentPtr FragmentWindow::GetIdleFragment()  {
  std::lock_guard<std::mutex> lock(lock_);
  if (free_fragments_.empty()) 
  {
    return std::make_shared<Fragment>();
  }
  else
  {
    auto p = free_fragments_[0];
    free_fragments_.erase(free_fragments_.begin());
    return p;
  }
}
 
const FragmentPtr &FragmentWindow::GetFragmentByName(const std::string &name) {
  std::lock_guard<std::mutex> lock(lock_);
  for (auto &fragment : fragments_)
  {
    if(fragment->FileName() == name)
    {
      return fragment; 
    }
  }
  return fragment_null;
}
 
std::string FragmentWindow::GetPlayList()  {
  std::lock_guard<std::mutex> lock(lock_);
  return playlist_;
}
 
void FragmentWindow::Shrink()  {
  std::lock_guard<std::mutex> lock(lock_);
  int remove_index = -1;
  if(fragments_.size() > window_size_)
  {
    return ;
  }
  remove_index = fragments_.size() - window_size_;

  for(int i = 0; i < remove_index && !fragments_.empty(); i++)
  {
    auto p = *fragments_.begin();
    fragments_.erase(fragments_.begin());
    p->Reset();
    free_fragments_.emplace_back(std::move(p));
  }
}
 
void FragmentWindow::UpdatePlayList()  {
 
  std::lock_guard<std::mutex> lock(lock_);
  //一般三个起播
  if(fragments_.empty() || fragments_.size() < 3)
  {
    return ;
  }
  std::ostringstream ss;

  //固定头
  ss << "#EXTM3U\n#EXT-X-VERSION:3\n";
  //最大的切片时长
  int i = fragments_.size() > 5 ? fragments_.size() - 5 : 0;
  int j = i;
  int64_t max_duration = 0;
  for(; j < fragments_.size(); j ++)
  {
    max_duration = std::max(max_duration, fragments_[j]->Duration());
  }
  //转换成秒
  int32_t target_duration = (int32_t)ceil(max_duration / 1000.0);
  ss << "#EXT-X-TARGETDURATION:" << target_duration << "\n";
  //第一个切片的序号
  ss << "#EXT-X-MEDIA-SEQUENCE:" << fragments_[i]->SequenceNo() << "\n";
  ss.precision(3);//输出精度,小数点后保留三位
  //设置浮点数的输出格式为定点格式
  ss.setf(std::ios::fixed, std::ios::floatfield);
  
  for(; i < fragments_.size(); i ++)
  {
    ss << "#EXTINF:" << fragments_[i]->Duration() / 1000.0 << ",\n";
    ss << fragments_[i]->FileName() << "\n";
  }
  playlist_.clear();
  playlist_= ss.str();
  HLS_TRACE << "playlist:\n" << playlist_;
}
