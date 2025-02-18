#include "../base/Singleton.h"
#include <iostream>
using namespace tmms::base;

class A : public NonCopyable
{
  public:
    A() = default;
    ~A() = default;
    void print()
    {
      std::cout << " A !! " << std::endl;
    }
};
int main()
{
  auto a = Singleton<A>::Instance();
  a->print();
  return 0;
}
