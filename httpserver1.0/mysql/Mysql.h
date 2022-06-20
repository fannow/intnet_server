#ifndef _MYSQL_H__
#define _MYSQL_H__
#include<iostream>
#include<mysql/mysql.h>
#include<vector>
#include<string>
#include<queue>
#include<pthread.h>
#include"../code/Log.h"
using namespace std;
struct User{

    string host;      //ip地址
    string username;  //数据库用户名
    string password;  //密码
    string db;        //数据库
    int port;         //端口
    User(string host,string username ,string password,string db,int port)
        :host(host),username(username),password(password),db(db),port(port){}

};
class Mysql{
    private:
    static pthread_mutex_t lock1;
    pthread_mutex_t lock;       //互斥锁
    int index=10;             //连接池数量
    queue<MYSQL*> q;            //数据库队列
    int row;        //查询结果的行数
    int col;        //查询结果的列数
    MYSQL_RES*res;  //sql执行结果将保存在这个数据结构
    static   Mysql*ms;
    private:
       Mysql()=default;//生成默认的构造函数

    public:
       //单例模式
       static Mysql**GetMysql(){
       pthread_mutex_lock(&lock1); 
        if(ms==NULL){
           ms=new Mysql();
       pthread_mutex_unlock(&lock1); 
         }
           return &ms;
       }
    void SetIndex(int index){
        this->index=index;
    }
    /**
     * 创建mysql连接对象
     * */
    void Init(){
       // 互斥锁的初始化
       pthread_mutex_init(&lock,NULL);
        for(int i=0;i<index;i++){
            MYSQL *mysql=NULL;
            mysql=mysql_init(mysql);
            if(mysql==NULL){
               // LOG_ERROR(LOG_ROOT())<<mysql_errno(mysql)<<endl;
            }
            q.push(mysql);
        }
    }
    void AddMysql(){
        MYSQL *mysql=NULL;
        
        if(mysql_init(mysql)==NULL){
     //       LOG_ERROR(LOG_ROOT())<<mysql_errno(mysql)<<endl;
           return;
        }
        q.push(mysql);
    }
    /**
     * 用于获取链接
     *
     * */


    
    MYSQL * GetConnect(User*u){
        pthread_mutex_lock(&lock);
        //每次建立连接前都要检测连接池有没有连接如果没有了，就添加连接到连接池
        if(q.empty()){
            AddMysql();
        }
        //获取连接队列的头节点连接
        MYSQL *mysql=q.front();
        q.pop();    
        if( mysql_real_connect(mysql,u->host.c_str(),u->username.c_str(),u->password.c_str(),u->db.c_str(),u->port,NULL,0)==NULL){
   //bin         LOG_ERROR(LOG_ROOT())<<mysql_error(mysql)<<endl;
           return NULL;
        }
        pthread_mutex_unlock(&lock);
        return mysql;  

    }
    /*
     *用于插入sql语句执行
     * */
    void Insert(MYSQL*mysql,string sql){
       pthread_mutex_lock(&lock); 
        /**
         * 设置字符集
         * */
        
        if(mysql_set_character_set(mysql,"utf8")!=0){

 //           LOG_ERROR(LOG_ROOT())<<mysql_error(mysql)<<endl;
        }
       
        if(mysql_real_query(mysql,sql.c_str(),sql.size())!=0){

 //           LOG_ERROR(LOG_ROOT())<<mysql_error(mysql)<<endl;
        }
        pthread_mutex_unlock(&lock);
    }
    /**
     *执行sql语句将结果集返回给外部函数，由外部函数获取结果执行
     * */    
    void SelectSql(MYSQL* mysql,string sql){
       pthread_mutex_lock(&lock); 
        /**
         * 设置字符集
         * */
        if(mysql_set_character_set(mysql,"utf8")!=0){
           cout<<mysql_error(mysql);

  //          LOG_ERROR(LOG_ROOT())<<mysql_error(mysql)<<endl;
        }
        if(mysql_real_query(mysql,sql.c_str(),sql.size())!=0){
           cout<<mysql_error(mysql);

   //         LOG_ERROR(LOG_ROOT())<<mysql_error(mysql)<<endl;
        } 
       cout<<"查询结束"<<endl;     
      /*
       *将查询结果保存到res中
       * */
       res= mysql_store_result(mysql);
       if(res){
   //        LOG_ERROR(LOG_ROOT())<<"mysql_store_result error"<<endl;

       }
       /*
        *获取结果的行数
        * */

         row=mysql_num_rows(res);
        if(row){

   //        LOG_ERROR(LOG_ROOT())<<"mysql_store_result error"<<endl;
        }

        /*
         *获取列数
         * */
         col=mysql_num_fields(res);
        if(col){
    //       LOG_ERROR(LOG_ROOT())<<"mysql_store_result error"<<endl;
        }
        
        //查询结果函数由外部函数执行
       pthread_mutex_unlock(&lock); 
    }
    /**
     *用于关闭当前连接
     *关闭连接是由主线程连接不需要加锁
     * */
    void Close(MYSQL*mysql){
       //处理完关闭数据库连接
       mysql_close(mysql);
       //防止内存泄漏
       mysql_library_end();
       //处理完将释放完的连接的MYSQL对象重新加入到任务队列中
       mysql=mysql_init(mysql);
       q.push(mysql);
       //每次关闭连接后都要检测连接池中的连接对象数量是不是初始化的数量
       delMysql();
    }

    /*
     *获取查询结果
     * */
    MYSQL_RES *GetAns(){
    
        return res;
    }
    /**
     *用于使用完释放结果集
     * */
    bool ResDestory(){
        
       pthread_mutex_lock(&lock); 
       mysql_free_result(res);
        
       pthread_mutex_unlock(&lock); 
       return true;
    }
    int GetRow(){
        return row;
    }
    int GetCol(){
        return col;
    }
    /**
     * 检测连接池中的数量是不是大于初始化的数量，如果大于初始化的数量就销毁多余的mysql对象
     *
     * */
    void delMysql(){
        int num=q.size()-index;
        if(num>0){
            while(num--){
                mysql_close(q.front());
                q.pop();
                 //防止内存泄漏
                 mysql_library_end();
            }
        }
    }
    /*
     *销毁数据库连接池
     * */
    bool Destory(){
        while(!q.empty()){

            mysql_close(q.front());
            q.pop();
            //防止内存泄漏
            mysql_library_end();
        }
    return true;
    }
    ~Mysql(){
        ResDestory();
        Destory();
        
    } 
};
Mysql* Mysql::ms=NULL;
pthread_mutex_t Mysql::lock1=PTHREAD_MUTEX_INITIALIZER;//静态初始化
/**由MysqlRall管理Mysql对象
 * 因为Mysql对象是new出来的，所以在使用完要delete 但是又是会忘记delete 
 * 这里我们将Mysql交给一个类来管理 当使用时创建一个MysqlRall临时对象 
 * 当使用完临时对象自动调用析构函数自动释放mysql对象
 * */

class MysqlRall{
    private:
        Mysql*mysql;

    public:
        MysqlRall(Mysql**mysql):
            mysql(*mysql){}
        ~MysqlRall(){
            /*
             *断开连接池中的所有连接
             * */
            mysql->Destory();
            delete mysql;
        }
        Mysql*GetMysql(){
            return mysql;
        }
};
#endif
