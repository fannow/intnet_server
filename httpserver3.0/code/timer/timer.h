#ifndef __TIMER_H__
#define __TIMER_H__
#include<iostream>
#include<functional>
#include<chrono>
#include<time.h>
#include<vector>
#include<unordered_map>
using namespace std;
typedef function<void(int)>  cd;                //回调函数
typedef chrono::high_resolution_clock timer; //时间级别秒
typedef   timer::time_point timeparse;                 //当前时间
typedef  chrono::milliseconds MS;
struct TimeNode{
    int id;                 //套接字
    cd cp;                 //回调函数
    timeparse newtime;
    timer timeout;        //时间毫秒级别
    TimeNode(int id,cd cp,timeparse time):
                id(id),cp(cp),newtime(time) {}
    bool operator<( TimeNode &tns){
      return  newtime<tns.newtime;//以设置定时时间排序先处理但是时间快到的任务
    }
};
class Timer{
private:
    vector<TimeNode*> tn;//时间节点
    unordered_map<int ,size_t>root; //键值对 节点id 下标index   按照下标 数组比较大小调整数组下标 
public:
    Timer();
    void dowork(int ip);
    void  AddNode(int id,int  timeout ,cd cp);    //添加节点
    void DelNode(int id);     //删除超时节点
    void Adjuet(int id,int timeout);      //调整 指定id的节点
    void pop();         //删除顶节点
    void Swap(size_t i,size_t j);
    void AdjustUp(int index);
    bool AdjusDown(int index,int n);
    void Tick();     //心搏函数---循环检测每个节点的定时时间是不是超时超时的节点就要处理
    int gettick();  //实际处理超时节点函数 
    void clear();    //清空节点
    ~Timer(){
     clear();
     }
};
#endif
