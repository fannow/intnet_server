#ifndef __SOKCET_H__
#define __SOCKET_H__
#include<iostream>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<unistd.h>
#include<algorithm>
#include<cstring>
#include<sys/stat.h>
#include<vector>
#include<unistd.h>
#include<unordered_map>
#include<sys/sendfile.h>
#include<sys/types.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/wait.h>
#include"Log.h"
#define NUM 32 
using namespace std;
class Sock{
public:
  Sock(){}
  static int SocKet(){
    int lsock=socket(AF_INET,SOCK_STREAM,0);
    if(lsock<0){
      LOG_FATAL(LOG_ROOT())<<"socket errer";
      exit(1);
    }
    return lsock;
  }
  static void Bind(int lsock,int port){
    struct sockaddr_in in_addr;
    in_addr.sin_family=AF_INET;
    in_addr.sin_port=htons(port);
    in_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    int bd=bind(lsock,(struct sockaddr*)&in_addr,sizeof(in_addr));
    if(bd<0){
      LOG_FATAL(LOG_ROOT())<<"bind errer";
      exit(2);
    }
  }
  static void Listen(int lsock){
    int lt=listen(lsock,32);
    if(lt<0){
      LOG_FATAL(LOG_ROOT())<<"listen errer";
      exit(3);
    }
  }
  static void SetSockOpt(int sock){
    int opt=1;
   int sso= setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
   if(sso<0){
      LOG_WARN(LOG_ROOT())<<"socketopt errer";
   }     
  }
  static int Accept(int lsock){
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    int sock=accept(lsock,(struct sockaddr*)&addr,&len);
    if(sock<0){
      LOG_FATAL(LOG_ROOT())<<"accept errer";
    }
      LOG_INFO(LOG_ROOT())<<"get a new  line ..";
      return sock;
  }
  static void getline(int sock,string &line,int *mepoll){
    char ch='a';
    //按字符读取
    //结尾不固定 \r 或\r\n或 \n 都要转换为 \n
    while(ch!='\n'){
      int s=recv(sock,&ch,1,0);
      if(s>0){
        if(ch=='\r'){

            recv(sock,&ch,1,MSG_PEEK);
            //将数据从内核缓冲区不拿出来而去
            if(ch=='\n'){
              //这里是结尾时\r\n 但是 \n还没有读出来，再读一遍
              recv(sock,&ch,1,0);
            }else{
              ch='\n';
            }
        }
        else if (s == 0) {
        //读取失败
            close(sock);
            epoll_ctl(*mepoll, EPOLL_CTL_DEL, sock, NULL);
            
            LOG_INFO(LOG_ROOT())<<"client close";
        }
        //这里读出的是正常字符,也可能时\n
        if(ch!='\n'){
          line.push_back(ch);

      }
    }
  }
}};
#endif
