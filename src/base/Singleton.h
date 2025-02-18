#pragma once
#include "NonCopyable.h"
#include <pthread.h>
namespace tmms
{
  namespace base
  {
      template <typename T>
      //单例模式
      class Singleton : public NonCopyable
      {
        public:
          Singleton() = delete;
          ~Singleton() = delete;
          //全局访问点函数
          static T* Instance()
          {
            pthread_once(&ponce_, &init);
            return value_;
          }
        private:
          static void init()
          {
            if(!value_)
            {
              value_ = new T();
            }
          }
          static pthread_once_t ponce_;
          static T* value_;
      };
      //静态变量类外初始化
      template <typename T>
      //该变量必须用PTHREAD_ONCE_INIT宏来静态初始化
      pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;
      template <typename T>
      T* Singleton<T>::value_ = nullptr;
  }
}
