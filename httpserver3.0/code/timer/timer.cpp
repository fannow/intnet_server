#include"timer.h"
Timer::Timer(){}
void Timer::AddNode(int id,int timeout,cd cp){
    if(root.count(id)==0){
        int index=tn.size();
        timeparse tims=timer::now()+MS(timeout);
        TimeNode *t=new TimeNode(id,cp,tims);;
        tn.push_back(t);
        root[id]=index;
        AdjustUp(index);
        //节点不存在添加节点
    }else if(root.count(id)){
        //索引存在就说明存在之前存在id的套接字替换套接字中的数据

        //节点存在替换节点中的数据
        int index=root[id];//找到节点在数组中的位置
        //替换节点中的数据
        tn[index]->id=id;
        tn[index]->cp=cp;
        tn[index]->newtime=timer::now()+MS(timeout);
        //修改节点影响定时时间，导致下边节点结构不符合
        if(! AdjusDown(index,tn.size())){
               AdjustUp(index);
        }
    }

}
bool Timer::AdjusDown(int index,int n){
    int root=index;
    int left=root*2+1;//左子节点
    while(left<n){
        if(left+1<n&&tn[left]>=tn[left+1]){
            left++;//找到字节点比较小的节点
        }
        if(tn[root]>tn[left]){
            //父节点比子节点大不符合小堆的交换节点
            Swap(root,left);
            //更新坐标
            root=left;
            left=root*2+1;
        }else{
            break;
        }

    }
    return root>index;
    
}
void Timer::AdjustUp(int index){
    int left=index;
    int root=(left-1)/2;
    while(root>=0){
        //每一个节点判断是不是符合小堆的结构
        if(tn[left]<tn[root]){
            //交换节点
            Swap(left,root);
            left=root;
            root=(left-1)/2;
        }else{
            break;

        }
    }
}

/**
 *删除指定节点并触发回调函数
 * */

void Timer::dowork(int  ip){
    if(tn.empty())
        return;
    if(root.count(ip)){
        //删除节点存在
        int index=root[ip];
        TimeNode *re=tn[index];
        
        //删除节点存在
        DelNode(index);
        //触发回调函数
        re->cp(re->id);
    }

}
/**
 *交换节点
 * */
void Timer::Swap(size_t i,size_t j){
    //先交换索引在交换数据
    root[tn[i]->id]=i;
    root[tn[j]->id]=j;
    TimeNode *rs=tn[i];
    tn[i]=tn[j];
    tn[j]=rs;
}
//删除指定位置节点
void Timer::DelNode(int index){
    if(tn.empty())
        return;
    //交换最后一个和删除位置的节点
    //最后将最后一个节点删除
    int id=tn[index]->id;
    int n=tn.size()-1;
    if(index<=n){
         Swap(index,n);
       // tn.pop_back();//删除最后一个节点
        if(!AdjusDown(index,n-1)){
         AdjustUp(index);
        }
    }
    tn.pop_back();//删除最后一个节点
    //删除对应的存储在map中的索引
    root.erase(id);

}

/**
 *
 * 调整指定id的节点的定时时间
 * */
void Timer::Adjuet(int id ,int timeout){
    tn[root[id]]->newtime=timer::now()+MS(timeout);
    AdjusDown(root[id],tn.size());
} 
void Timer::pop(){
    DelNode(0);
}
/**
 * 心搏函数
 * */
void Timer::Tick(){
   if(tn.empty())
        return;
   while(!tn.empty()){
       TimeNode*ts=tn.front();
       if(std::chrono::duration_cast<MS>(ts->newtime - timer::now()).count() > 0){
           //没有超时
           //跳出循环
           break;
       }
      // 超时节点--删除超时节点--执行回调函数
        ts->cp((ts->id));
        DelNode(0);
   }
}
/*
 *处理超时节点-----并执行回调函数
 * */
int Timer::gettick(){
    Tick();
    size_t res=-1;
    if(!tn.empty()){
         res = std::chrono::duration_cast<MS>(tn.front()->newtime - timer::now()).count();
        if(res < 0)
            res = 0;
    }
        return res;

}
void Timer::clear(){
    tn.clear();
    root.clear();
}
