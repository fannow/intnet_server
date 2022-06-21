#include<iostream>
#include<strings.h>
#include<stdio.h>
#include<string>
#include<unistd.h>
#include<stdlib.h>
#include"../mysql/Mysql.h"
#include"../code/Log.h"
using namespace std;
void fun(string post){
  size_t pos=post.find('&');
  string port1;
    string port2;
  if(pos!=string::npos){
      port1=post.substr(0,pos);
      port2=post.substr(pos+1);      
  }
    size_t pos1=port1.find('=');
    size_t pos2=port2.find('=');
    int a=0;
    int b=0;
    if(pos1!=string::npos){
      a=atoi(port1.substr(pos1+1).c_str());
    }
    if(pos2!=string::npos){
       b=atoi(port2.substr(pos2+1).c_str()); 
    }
cout<<a<<"+"<<b<<"="<<a+b<<endl;
cout<<a<<"-"<<b<<"="<<a-b<<endl;
cout<<a<<"/"<<b<<"="<<a/b<<endl;
cout<<a<<"*"<<b<<"="<<a*b<<endl;



}
/**
 *
 * get参数--管道
 * host--环境变量
 * */
string func(){
    string method="";
    string env="";
    if(getenv("METHOD")){
         method=getenv("METHOD");
    }else{

        LOG_ERROR(LOG_ROOT())<<"cgi getenv error"<<endl;
        return NULL;
    }

  if(strcasecmp(method.c_str(),"get")==0){
    env=getenv("QUERY_STRING");
  }else if(strcasecmp(method.c_str(),"post")==0){
    size_t s=atoi(getenv("CONTENT-LENGTH"));    
    char c;
   while(s){
      read(0,&c,1);
      env.push_back(c);
      s--;
    }
  }
  return  env;
}
/**
 *数据库处理
 *
 * */
void mysql(string post){
    
    MysqlRall mr(Mysql::GetMysql());
    Mysql*m=mr.GetMysql();
    m->SetIndex(10);
    m->Init();
    User*u=new User("127.0.0.1","root","254819li","db",3306);
    MYSQL*mysql=m->GetConnect(u);
    
     size_t pos=post.find('&');
     string port1;
     string port2;
    if(pos!=string::npos){
      port1=post.substr(0,pos);
      port2=post.substr(pos+1);      
     }
    size_t pos1=port1.find('=');
    size_t pos2=port2.find('=');
    int a=0;
    int b=0;
    if(pos1!=string::npos){
      a=atoi(port1.substr(pos1+1).c_str());
    }
    if(pos2!=string::npos){
       b=atoi(port2.substr(pos2+1).c_str()); 
    }
    string sql="insert into st(id,name) values (";
    sql+=to_string(a);
    sql+=",";
    sql=sql+"\'"+to_string(b)+"\');";

       LOG_DEBUG(LOG_ROOT()) <<"sql"<< sql<<endl;
    m->Insert(mysql,sql);
    m->Close(mysql);
    
    cout<<post<<endl;
}
int main(){

    string env=func();
    LOG_DEBUG(LOG_ROOT())<<env;
    mysql(env);

}
