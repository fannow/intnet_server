#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__
#include"http.h"
#include"./pthreadpool/pp.h"
#define EPOLL_SIZE 10240
struct DO{
    Make*make=NULL;
    int *mepoll;
    int sock;
    struct epoll_event event;
    DO(int sock,int *mepoll,Make*make,struct epoll_event event):make(make),sock(sock),mepoll(mepoll),event(event) {}
    void Run(){
        make->Run();
        event.events=EPOLLOUT;
        epoll_ctl(*mepoll, EPOLL_CTL_MOD, sock, &event);
    }
};
//缓冲区结构体
struct  buter {
    int sock;
    request *re; 
    Make *make;
    response *rs;
   

    buter() {}
    buter(int sock_) :sock(sock_), re(new request(sock))
    {
        re = new request(sock);
        make = new Make(re);
        rs = new response(sock, make->GetRequset(), make);
    }

};
class HttpServer{
private:
  int port;
  int lsock;
  int mepoll;
 threadpool<DO> *pp=NULL;
  static HttpServer*httpserver;
  static pthread_mutex_t lock;
  static pthread_mutex_t lock1;
  HttpServer(int _port):port(_port),lsock(-1)
  {}
  int index=0;
public:
  void Start(){
    struct epoll_event eve[EPOLL_SIZE];//就绪事件集合,有epoll_create返回就绪事件集合
      //将监听套接字对应的写事件添加到对应的epoll模型中
     AddEpoll(lsock,EPOLLIN);
     while(true){
        LOG_DEBUG(LOG_ROOT())<<"epoll wait";
         int num = epoll_wait(mepoll, eve, EPOLL_SIZE, -1);
         if((num<0)&&(errno!=EINTR)){
             LOG_ERROR(LOG_ROOT())<<"epoll failure";
         }else if(num==0){
             LOG_ERROR(LOG_ROOT())<<"wait outtime!";
         }else{
        LOG_DEBUG(LOG_ROOT())<<"epoll server";
            Service(eve,num);
         }

     }
  }
  
  void Init(){
	pp=new threadpool<DO>(10,1000);
    lsock=Sock::SocKet();
    Sock::SetSockOpt(lsock);//端口复用
    Sock::Bind(lsock,port);
    Sock::Listen(lsock);
   //创建epoll模型
   mepoll =epoll_create(128);
   if(mepoll<0){
       LOG_ERROR(LOG_ROOT())<<"epoll_create error";
     exit(5);
   }
  }
  //单例模式
  static HttpServer*GetInstance(int sk){
    if(httpserver==NULL){
     pthread_mutex_lock(&lock);
     if(NULL==httpserver){
     httpserver=new HttpServer(sk);
     pthread_mutex_unlock(&lock);

     }
    }
    return httpserver;
  }


  void Service(struct epoll_event event[], int num) {
      for (int i = 0;i < num;i++) {
          uint32_t en = event[i].events;
              struct buter* bt= (struct buter*)event[i].data.ptr;
              if (event[i].data.ptr == NULL&&en&EPOLLIN) {
                      int sock = Sock::Accept(lsock);
                      //将新建套接字以写时建添加到epoll模型中
                      AddEpoll(sock, EPOLLIN);
                    
              }else if(en&(EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                LOG_ERROR(LOG_ROOT())<<"system bug close connect";
                close(bt->sock);
              }else if (event[i].data.ptr != NULL&&en&EPOLLIN) {
                         //普通读事件
                       request* re = bt->re;
                       //request的收取
                        //传入epoll模型如果读取失败就把该套接字从mepoll删除
                      re->Run(&mepoll);
                      //将任务处理处理加入到线程池中
						DO* dos=new DO(bt->sock,&mepoll,bt->make,event[i]);
						pp->append(dos);
              }
              else if (en & EPOLLOUT) {
                  bt->rs->Run();
                  close(bt->sock);
                
                epoll_ctl(mepoll, EPOLL_CTL_DEL, bt->sock, NULL);
              }
              else {
              //其他事件
              
              }

          }
     }
  //将套接字添加到epoll模型中
void  AddEpoll(int sock,uint32_t opev) {
    //套接字对应的事件添加到epoll模型中
      struct epoll_event   event;
      //设置是按对应的字段；
      event.events = opev;

      //监听套接字只需要负责连接事件所以不需要缓冲区将缓冲区设置为null
      if (sock == lsock) {
          event.data.ptr = NULL;
      
      }
      else {
        //其他套接字设置缓冲区
          struct buter *bt= new struct buter(sock);
      
          event.data.ptr = bt;
             
      }
      epoll_ctl(mepoll, EPOLL_CTL_ADD, sock, &event);
  }
  ~HttpServer(){
    if(lsock>-1){
      close(lsock);
    }
    close(mepoll);
  }

};
HttpServer*HttpServer::httpserver=NULL;
pthread_mutex_t HttpServer::lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t HttpServer::lock1=PTHREAD_MUTEX_INITIALIZER;
#endif
