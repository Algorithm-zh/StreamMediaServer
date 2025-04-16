#include "TTime.h"
#include "StringUtils.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace tmms::base;

void TestTTime()
{
    std::cout <<" now:"<<tmms::base::TTime::Now() << " time:"<< time(NULL) << " now ms:" << tmms::base::TTime::NowMS() << std::endl;

}
void TestString1()
{
    const std::string str2 = "";
    const std::string str3 = "a";    
    const std::string str4 = "aa;ab;ac;ad;ae;";
    const std::string str5 = ";;;;;";
    char de = ';';
    std::vector<std::string> list = tmms::base::StringUtils::SplitStringWithFSM(str4,de);
    std::cout <<"delimiter:"<<de <<" str4:" << str4 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }

    list = tmms::base::StringUtils::SplitStringWithFSM(str5,de);
    std::cout <<"delimiter:"<<de <<" str5:" << str5 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }


    list = tmms::base::StringUtils::SplitStringWithFSM(str3,de);
    std::cout <<"delimiter:"<<de <<" str3:" << str3 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }

    list = tmms::base::StringUtils::SplitStringWithFSM(str2,de);
    std::cout <<"delimiter:"<<de <<" str2:" << str2 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }      
}
void TestString()
{
    const std::string str = "abcdadcb";
    const std::string str1 = "aaaaaa";
    const std::string str2 = "";
    const std::string str3 = "a";
    const std::string str4 = "aa;ab;ac;ad;ae;";
    const std::string str5 = ";;;;;";
    const std::string start = "abc";
    const std::string start1 = "abca";
    const std::string start2 = "";
    const std::string de = ";";

    std::cout <<"start:"<<start <<" str:" << str << " result:"<<  tmms::base::StringUtils::StartsWith(str,start) << std::endl;
    std::cout <<"start1:"<<start1 <<" str:" << str << " result:"<<  tmms::base::StringUtils::StartsWith(str,start1) << std::endl;
    std::cout <<"start2:"<<start2 <<" str:" << str << " result:"<<  tmms::base::StringUtils::StartsWith(str,start2) << std::endl;
    std::cout <<"start:"<<start <<" str1:" << str1 << " result:"<<  tmms::base::StringUtils::StartsWith(str1,start) << std::endl;
    std::cout <<"start:"<<start <<" str2:" << str2 << " result:"<<  tmms::base::StringUtils::StartsWith(str2,start) << std::endl;
    std::cout <<"start:"<<start <<" str3:" << str3 << " result:"<<  tmms::base::StringUtils::StartsWith(str3,start) << std::endl;

    std::cout <<"end:"<<start <<" str:" << str << " result:"<<  tmms::base::StringUtils::EndsWith(str,start) << std::endl;
    std::cout <<"end1:"<<start1 <<" str:" << str << " result:"<<  tmms::base::StringUtils::EndsWith(str,start1) << std::endl;
    std::cout <<"end2:"<<start2 <<" str:" << str << " result:"<<  tmms::base::StringUtils::EndsWith(str,start2) << std::endl;
    std::cout <<"end:"<<start <<" str1:" << str1 << " result:"<<  tmms::base::StringUtils::EndsWith(str1,start) << std::endl;
    std::cout <<"end:"<<start <<" str2:" << str2 << " result:"<<  tmms::base::StringUtils::EndsWith(str2,start) << std::endl;
    std::cout <<"end:"<<start <<" str3:" << str3 << " result:"<<  tmms::base::StringUtils::EndsWith(str3,start) << std::endl;

    std::vector<std::string> list = tmms::base::StringUtils::SplitString(str4,de);
    std::cout <<"delimiter:"<<de <<" str4:" << str4 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }

    list = tmms::base::StringUtils::SplitString(str5,de);
    std::cout <<"delimiter:"<<de <<" str5:" << str5 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }


    list = tmms::base::StringUtils::SplitString(str3,de);
    std::cout <<"delimiter:"<<de <<" str3:" << str3 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }

    list = tmms::base::StringUtils::SplitString(str2,de);
    std::cout <<"delimiter:"<<de <<" str2:" << str2 << " result:" << list.size() << std::endl;
    for(auto v:list)
    {
        std::cout << v << std::endl;
    }    
}

// void TestTask()
// {
//     TaskPtr task1 = std::make_shared<Task>([](const TaskPtr &task)
//                                         {
//                                             std::cout << "task1 interval = "<< 1000
//                                                     << " now:" << TTime::NowMS()<< std::endl;
//                                         },
//                                         1000);
//     TaskPtr task2 = std::make_shared<Task>([](const TaskPtr &task)
//                                         {
//                                             std::cout << "task2 interval = "<< 500
//                                                     << " now:" << TTime::NowMS()<< std::endl;
//                                             task->Restart();
//                                         },
//                                         500);    
//     TaskPtr task3 = std::make_shared<Task>([](const TaskPtr &task)
//                                         {
//                                             std::cout << "task3 interval = "<< 60000
//                                                     << " now:" << TTime::NowMS()<< std::endl;
//                                             task->Restart();
//                                         },
//                                         60000);  
//     TaskPtr task4 = std::make_shared<Task>([](const TaskPtr &task)
//                                         {
//                                             std::cout << "task4 interval = "<< 1000
//                                                     << " now:" << TTime::NowMS() << std::endl;
//                                             task->Restart();
//                                         },
//                                         1000);
//     sTaskMgr->Add(task1);  
//     sTaskMgr->Add(task2); 
//     sTaskMgr->Add(task3); 
//     sTaskMgr->Add(task4);                                                                                                                                                               
// }
int main(int argc,const char **argv)
{
    TestString1();
    return 0;
}