#ifndef __LOG_H__
#define __LOG_H__
#include<iostream>
#include<memory>
#include<list>
#include<vector>
#include<stdint.h>
#include<cstdarg>
#include<map>
#include<sstream>
#include<unordered_map>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include<pthread.h>
#include<unistd.h>
#include<sstream>
#include <sys/syscall.h>
#include"../mysql/Mysql.h"
#include"smtp.h"
using namespace std;
#define readfile "../txt/smtp.txt"
#define smtp_flag 0
pid_t GetThreadId() {
    return syscall(SYS_gettid);
}
/*
* ���ڿ���̨�����
*/
#define LOG_LEVEL(logger,level) \
    if(logger->GetLevel() <= level) \
			LogEventWarp(LogEvent::ptr(new LogEvent(logger, level,__FILE__,\
			__LINE__, 0, GetThreadId(),2, time(0)))).getSS()
#define LOG_DEBUG(logger) LOG_LEVEL(logger, LogLevel::DEBUG)
#define LOG_ERROR(logger) LOG_LEVEL(logger, LogLevel::ERROR)
#define LOG_FATAL(logger) LOG_LEVEL(logger, LogLevel::FATAL)
#define LOG_INFO(logger) LOG_LEVEL(logger, LogLevel::INFO)
#define LOG_WARN(logger) LOG_LEVEL(logger, LogLevel::WARN)

#define LOG_ROOT()  LogRoot::GetInstance()->getroot()
#define LOG_NAME(name) LogRoot::GetInstance()->getlogger(name);
class Logger;
class LogLevel {
public:
    typedef shared_ptr<LogLevel>ptr;
    enum Level {
        UNKNOW,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };
    static  string ToString(LogLevel::Level level);
    static LogLevel::Level FormString(string str);
};

class LogEvent {
private:
    const char* file_name;           //��־������
    uint32_t line;                  //�к�
    uint32_t m_time;                //��������ʱ��
    uint32_t threadid;              //�߳�id��
    uint32_t fiberid;               //Э��id
    uint32_t t_time;                //�¼���
    shared_ptr<Logger> logger;
    stringstream sss;
    LogLevel::Level level;
public:
    typedef shared_ptr<LogEvent>ptr;
    LogEvent(shared_ptr<Logger> logger, LogLevel::Level level, const char* name, uint32_t line,
            uint32_t m_time, uint32_t threadid, uint32_t fiberid, uint32_t t_time);
    const char* GetName() { return file_name; }
    uint32_t GetLine() { return line; }
    uint32_t GetTime() { return m_time; }
    uint32_t GetThreadId() { return threadid; }
    uint32_t GetFiberId() { return fiberid; }
    shared_ptr<Logger> GetLogger() { return logger; }
    LogLevel::Level GetLevel() { return level; }
    uint32_t GetNowTime() { return t_time; }
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list va);
    stringstream& getss() { return sss; }
    string getSS() { return sss.str(); }
};

class Formater {
public:
    Formater(string str) :
        formate(str)
    {
       
        Init();//�������ʱֱ�ӽ���־��ʽ����־��ʽ��Ԫ��Ӧ����
    }
    typedef shared_ptr<Formater> ptr;
    class Item {
    public:
        typedef shared_ptr<Item>ptr;
        virtual  void  format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        ~Item() {}
    };
    string format(shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)//ѭ��������־���Ƶ�Ԫ����ʽ��������һ�ַ�����ʽ������һ��
    {
        stringstream  ss;
        for (size_t i = 0;i < m_items.size();i++) {
            //������־��ʽ���Ƶ�Ԫ��formdar������д��ss ���ظ���־�����������־�����
            m_items[i]->format(ss, logger, level, event);
        }
        return ss.str();
        
    }
        
     void Init();//������־��ʽ ����Ӧ����־��ʽ����־��ʽ���Ƶ�Ԫ��Ӧ����
    string GetFormate() { return formate; }
    bool IsError() { return isno; } //�����־��ʽ�ǲ����д���
public:
    bool isno;
    string formate;//�û��������־
    vector<Item::ptr> m_items;
};
class LogAppender {
public:
    typedef  shared_ptr<LogAppender>ptr;
    virtual void log(shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event) = 0;
    //������־������Լ�����־��ʽ
    void SetFormate(Formater::ptr formate) {
        this->formate = formate;
        if (formate) {
            has = true;
        }
        else {
            has = false;
        }

    }
    Formater::ptr GetFormat() { return formate; }
    LogLevel::Level GetLevel() { return level; }
    string GetFormate() { return  formate->GetFormate(); }
protected:

    bool has;
    Formater::ptr formate;
    LogLevel::Level level;
};
#include<fstream>
class FileAppender :public LogAppender {
public:
    typedef shared_ptr<FileAppender>ptr;
    FileAppender(const char* fileA) :
        file_name(fileA) {
        reopen();

    }
    void log(shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event);
    bool reopen();//����־�ļ�
private:
    const char* file_name;
    ofstream file_stream;
};

class StdoutAppender :public LogAppender {
public:
    typedef shared_ptr<StdoutAppender> ptr;

    void log(shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event);
};
//���������
class SmsAppender:public LogAppender{

    private:
          unordered_map<string ,string> phone;
    public:
          typedef shared_ptr<SmsAppender> ptr;
          SmsAppender(){
              phone["root"]="17609275732";
          }
          void AddPhone(string name,string _phone);
          void DelPhone(string name);
          void ClearPhone(){phone.clear();}
          void Socket();//���𴴽�����
          //���ŷ���
          void log(shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event);
 };
//�ʼ������
class SmtpAppeder:public LogAppender{
  private:
  string smtpIp;     //����������
  string formail;  //��������
  string posswd;    //����
  unordered_map<string ,string> tomail;   //�ռ�������
  string toil;             //�ʼ�����
  string mail;             //�ʼ�����
  public:
  SmtpAppeder(string IP,string fmail,string po,unordered_map<string,string> tm):
    smtpIp(IP),formail(fmail),posswd(po),tomail(tm){}
  void AddToMail(string name,string mail){tomail[name]=mail;}
  void DelToMail(string name){tomail.erase(name);}
  //ɾ��
  void ClearToMail(){tomail.clear();}
  void Read();//��ȡ�����ļ�����
  //�ʼ�����
  void log(shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event);
};
class MysqlAppender:public LogAppender{

    private:
        string db="";
        User *u=NULL;
        Mysql*m=NULL;
        MYSQL* mysql=NULL;
    public:
        MysqlAppender(string ip,string username,string passwd,string  db,int port){
            u=new User(ip,username,passwd,db,port);
           this-> db=db;
            ConnectMysql();
        }
        void ConnectMysql();
         void log(shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event);
};
class Logger :public enable_shared_from_this<Logger> {
public:
    Logger(string name = "root"):
    level(LogLevel::DEBUG) {
        this->name = name;
        //����Ĭ����־��ʽ
        formate.reset(new Formater("%d%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m %n"));

    }
    typedef shared_ptr<Logger>ptr;
    void AddAppender(LogAppender::ptr app);
    void delAppedner(LogAppender::ptr app);
    void CleanAppender() { appenders.clear(); }
    string GetLoggerName() { return name; }
    void SetFormat(Formater::ptr formate);
    void SetStringFormat(string str);
    void SetLogger(Logger::ptr logger) { root = logger; }
    string GetFormate() { return formate->GetFormate(); }
    void SetLevel(LogLevel::Level level) { this->level = level; }
    void log(LogLevel::Level level, LogEvent::ptr event);
    LogLevel::Level GetLevel() { return level; }
private:
    string name;
    Logger::ptr root;
    Formater::ptr formate;
    LogLevel::Level level;
    list<LogAppender::ptr> appenders;
};

class LoggerMes {
public:
    typedef shared_ptr<LoggerMes> ptr;
    LoggerMes() {
        //����Ĭ����־�������뵽��־��������
        root.reset(new Logger());
        //���Ĭ����־�����
        unordered_map<string ,string >mps;
        mps["���"]="1702959373@qq.com";
        root->AddAppender(LogAppender::ptr(new StdoutAppender()));
        //����
        //root->AddAppender(LogAppender::ptr(new SmtpAppeder("smtp.qq.com","185334690@qq.com","ysidtuhpbiwxbhdg",mps)));//�ʼ�
        //root->AddAppender(LogAppender::ptr(new MysqlAppender("127.0.0.1","root","254819li","db",3306)));

        mp[root->GetLoggerName()] = root;
    }
    Logger::ptr getlogger(string name) {
        auto it = mp.find(name);
        if (it != mp.end()) {
            return it->second;
        }
        //�����ڴ���һ��logger
        Logger::ptr logger(new Logger(name));
        logger->SetLogger(root);
        mp[name] = logger;
        return logger;
    }
    Logger::ptr getroot() { 
        return root; 
    }
private:
    Logger::ptr root;
    unordered_map<string, Logger::ptr> mp;
};
/*
*
* LogEventWarp��Ϊһ����ʱ������ʹ�����ֱ��������������־ֱ��д�룬
*Ȼ����־����������ָ�룬��������������������棬���򲻽������޷��ͷ�
*/
class LogEventWarp {
public:

    LogEventWarp(LogEvent::ptr e)
        :m_event(e) {}
    ~LogEventWarp() {
        m_event->GetLogger()->log(m_event->GetLevel(), m_event);
    }
    shared_ptr<LogEvent>	getevent() { return m_event; }
    //��ȡ��־������
    stringstream& getSS() { return m_event->getss(); }
private:
    //��־�¼�
    shared_ptr<LogEvent> m_event;


};
template<class T, class K = void>
class Signal {
public:
    static T* GetInstance() {

        static T v;
        return &v;
    }
};
typedef Signal<LoggerMes> LogRoot;
#endif
