#include"Log.h"
/*
 *实现日志器事件
 *
 * */
LogLevel::Level LogLevel::FormString(string str) {
#define XX(level,v)\
    if(str==#v){\
      return LogLevel::level;\
    }

	XX(DEBUG, debug);
	XX(INFO, info);
	XX(WARN, warn);
	XX(ERROR, error);
	XX(FATAL, fatal);
	XX(DEBUG, DEBUG);
	XX(INFO, INFO);
	XX(WARN, WARN);
	XX(ERROR, ERROR);
	XX(FATAL, FATAL);
	return LogLevel::UNKNOW;
#undef XX
}
string LogLevel::ToString(LogLevel::Level level) {
	switch (level) {
#define XX(name)\
	case name:\
	return #name;\

		XX(DEBUG);
		XX(WARN);
		XX(INFO);
		XX(ERROR);
		XX(FATAL);
#undef XX
	default:
		return "UNKNOW";
	}
	return "UNKNOW";
}


/*
 *
 * 日志事件实现
 * */
LogEvent::LogEvent(shared_ptr<Logger>logger, LogLevel::Level level, const char* name, uint32_t line, uint32_t m_time, uint32_t threadid, uint32_t fiberid, uint32_t t_time) :
	file_name(name), line(line), m_time(m_time), threadid(threadid), fiberid(fiberid),
	t_time(t_time), logger(logger), level(level) {}
void LogEvent::format(const char* fmt, ...) {
	va_list al;
	va_start(al, fmt);
	format(fmt, al);
	va_end(al);
}
void LogEvent::format(const char* fmt, va_list al) {
	char* buff = NULL;
	int len = vasprintf(&buff, fmt, al);

	if (len != -1) {
		sss << string(buff, len);

	}
	delete buff;
}

/*
日志控制单元实现
**/

//消息控制单元

class Messageitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	Messageitemforma(string str = "") {}

	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->getSS();
	}

};
//日志级别控制单元
class Levelitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	Levelitemforma(string str = "") {}

	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << LogLevel::ToString(level);
	}

};
//程序启动后消耗时间
class Pleaseitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	Pleaseitemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetTime();
	}

};
//日志器名称
class lognameitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	lognameitemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetLogger()->GetLoggerName();
	}
};


//线程id
class ThreadIditemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	ThreadIditemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->GetThreadId();
	}
};
//协程id
class fiberiditemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	fiberiditemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetFiberId();
	}
};

//运行时间
class Timeitemforma :public Formater::Item {
private:
	//时间格式
	string time_formate;
public:
	typedef shared_ptr<Messageitemforma> ptr;
	Timeitemforma(string format = "%Y:%m:%d %H:%M:%S") :
		time_formate(format) {
		if (time_formate.empty()) {
			time_formate = "%Y:%m:%d %H:%M:%S";
		}
	}


	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		//日志时间格式设置
		struct tm* tm;
		time_t t = event->GetNowTime();
		tm = localtime(&t);
		char buff[64];
		//时间格式设置函数
		strftime(buff, sizeof(buff), time_formate.c_str(), tm);
		os << buff;
	}

};

//文件名称
class filenameitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	filenameitemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetName();
	}
};
//文件行号
class lineitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	lineitemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetLine();
	}
};


class stringitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	stringitemforma(string format) :
		str(format) {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << str;
	}
private:

	string str;
};
class tabitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	tabitemforma(string format) {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << '\t';
	}
};


class newlineitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	newlineitemforma(string format)
	{}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << endl;
	}
};
/*
日志格式控制实现

//*///string Formater::format(shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr evnt) {
//	stringstream  ss;
//	int i = 0;
//	for (int i = 0;i < m_items.size();i++) {
//		cout << "i" << endl;
//		//调用日志格式控制单元的formdar将数据写进ss 返回给日志输出器用于日志的输出
//		m_items[i]->format(ss, logger, level, evnt);
//	}
//	return ss.str();
//}
//将日志格式和日志格式控制单元对应起来插入到vector中
void Formater::Init() {
	//tuple(str,fmt,type)
	vector<tuple<string, string, int>>vec;

	string nstr;
	for (size_t i = 0;i < formate.size();i++) {
		if (formate[i] != '%') {
			nstr.append(1, formate[i]);
			continue;
		}
		//其实还是一个%
		if ((i + 1) < formate.size()) {
			if (formate[i + 1] == '%') {
				nstr.append(1, '%');
				continue;
			}
		}
		size_t n = i + 1;
		int fmt_staus = 0;
		int fmt_beign = 0;


		string str;
		string fmt;
		while (n < formate.size()) {
			//遇见空格
			if (!isalpha(formate[n]) && formate[n] != '}' && formate[n] != '{') {
				str = formate.substr(i + 1, n - i - 1);
				break;
			}
			if (fmt_staus == 0) {
				if (formate[n] == '{') {
					//字符串分割
					str = formate.substr(i + 1, n - i - 1);
					fmt_staus = 1;
					fmt_beign = n;
					n++;
					continue;
				}

			}
			else if (fmt_staus == 1) {
				if (formate[n] == '}') {
					//字符串分割
					fmt = formate.substr(fmt_beign + 1, n - fmt_beign - 1);
					//	cout << "#" << fmt << endl;
					fmt_staus = 0;
					++n;
					break;
				}

			}
			++n;
			if (n == formate.size()) {
				if (str.empty()) {
					str = formate.substr(i - 1);
				}
			}
		}

		if (fmt_staus == 0) {
			if (!nstr.empty()) {
				vec.push_back(make_tuple(nstr, string(), 0));
				nstr.clear();
			}
			str = formate.substr(i + 1, n - i - 1);

			//解析日志将日志存储在数组中

			vec.push_back(make_tuple(str, fmt, 1));
			i = n - 1;
		}
		else if (fmt_staus == 1) {
			isno = true;
			cout << "pattern parse error" << formate << "-" << formate.substr(i) << endl;
			//格式错误
			vec.push_back(make_tuple("<<pattern_error>>", fmt, 0));
		}
	}
	if (!nstr.empty()) {
		vec.push_back(make_tuple(nstr, "", 0));

	}

	/*
	* %m--->消息体
	* %p--->日志级别
	* %r--->启动的时间
	* %c--->日志名称
	* %t--->线程id
	* %n--->回车
	* %d--->时间
	* %f--->文件名
	* %l--->行号
	*/

	//function<Item::ptr(string str)> 返回值为iter::ptr 参数为string得函数
	static map<string, function<Formater::Item::ptr(string str)>> formar_item = {

#define xx(str,c) \
		{ #str,[](string fmt) {return Formater::Item::ptr(new c(fmt));}}

		xx(m, Messageitemforma),
		xx(p, Levelitemforma),
		xx(r, Pleaseitemforma),
		xx(c, lognameitemforma),
		xx(t, ThreadIditemforma),
		xx(n, newlineitemforma),
		xx(d, Timeitemforma),
		xx(f, filenameitemforma),
		xx(l, lineitemforma),
		xx(T, tabitemforma),
		xx(F, fiberiditemforma),

#undef xx

	};
	for (auto& i : vec) {
		if (get<2>(i) == 0) {
			m_items.push_back(Formater::Item::ptr(new stringitemforma(get<0>(i))));
		}
		else {
			auto it = formar_item.find(get<0>(i));
			if (it == formar_item.end()) {
				isno = true;
				m_items.push_back(Formater::Item::ptr(new stringitemforma("<<error_format %" + get<0>(i) + ">>")));
			}
			else {
				m_items.push_back(it->second(get<1>(i)));

			}

		}

	}


}
/*
 *文件输出器实现
 *
 * */
void FileAppender::log(shared_ptr<Logger> logger, LogLevel::Level le, LogEvent::ptr event) {
	//将数据写进日志文件
	//先测试文件是不是打开
	//要不是打开状态 重新打开文件
   // 将数写进日志文件
	if (le >= level) {
		file_stream << formate->format(logger, le, event);
	}

}

bool FileAppender::reopen() {
	if (!file_stream) {
		//先关闭文件再重新打开
		file_stream.close();
	}
	file_stream.open(file_name);
	return !!file_stream;
}
void StdoutAppender::log(shared_ptr<Logger>logger, LogLevel::Level le, LogEvent::ptr event) {
	if (le >= level) {
		cout << formate->format(logger, le, event);
	}

}
//增加短信接受人
void SmsAppender::AddPhone(string name,string _phone){
    phone[name]=_phone;

}
void SmsAppender::DelPhone(string name){
    if( phone.count(name)){
        phone.erase(name);
    }
}
#include<unistd.h>
#define SA struct sockaddr
#define MAXLINE 4096
#define MAXSUB  2000
#define MAXPARAM 2048
#define LISTENQ 1024

int basefd;
char *hostname = (char*)"106.ihuyi.com";
char *send_sms_uri = (char*)"/webservice/sms.php?method=Submit&format=json";
 
/**
* 发http post请求
*/
ssize_t http_post(char *page, char *poststr)
{
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    ssize_t n;
    snprintf(sendline, MAXSUB,
        "POST %s HTTP/1.0\r\n"
        "Host: %s\r\n"
        "Content-type: application/x-www-form-urlencoded\r\n"
        "Content-length: %zu\r\n\r\n"
        "%s", page, hostname, strlen(poststr), poststr);
 
    write(basefd, sendline, strlen(sendline));
    while ((n = read(basefd, recvline, MAXLINE)) > 0) {
        recvline[n] = '\0';
        printf("%s", recvline);
    }
    return n;
}
 
/**
* 发送短信
*/
ssize_t send_sms(char *account, char *password, char *mobile, char *content)
{
    char params[MAXPARAM + 1];
    char *cp = params;
    sprintf(cp,"account=%s&password=%s&mobile=%s&content=%s", account, password, mobile, content);
    return http_post(send_sms_uri, cp);
}
 

int  socked_connect(char *arg)
{
    struct sockaddr_in their_addr = {0};  
    char buf[1024] = {0};  
    char rbuf[1024] = {0};  
    char pass[128] = {0};  
    struct hostent *host = NULL;   
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd<0)
    {
        printf ("create the sockfd is failed\n");
        return -1;
    }
    
    if((host = gethostbyname(arg))==NULL)  
    {  
        printf("Gethostname error, %s\n");  
        return -1; 
    }  
 
    memset(&their_addr, 0, sizeof(their_addr));  
    their_addr.sin_family = AF_INET;  
    their_addr.sin_port = htons(80);  
    their_addr.sin_addr = *((struct in_addr *)host->h_addr);
    if(connect(sockfd,(struct sockaddr *)&their_addr, sizeof(struct sockaddr)) < 0)  
    {  
        close(sockfd);
        return  -1;
    }  
    printf ("connect is success\n");
    return sockfd;
    
}

void SmsAppender::Socket(){

}
 //短信发送
void SmsAppender::log(shared_ptr<Logger>logger, LogLevel::Level level, LogEvent::ptr event){
struct sockaddr_in servaddr;
    char str[50];
    
    for(auto it=phone.begin();it!=phone.end();it++){
    if((basefd= socked_connect(hostname))==-1)
    {
        printf("connect is failed\n");
        return;
    }
    printf("basefd is =%d\n",basefd);
    //查看用户名 登录用户中心->验证码通知短信>产品总览->API接口信息->APIID
     char *account = (char*)"C84353732";
 
    //查看密码 登录用户中心->验证码通知短信>产品总览->API接口信息->APIKEY
    char *password =(char*) "28e86569d167048485fdb898254db7da";
 
    //手机号
    char *mobile = (char*)it->second.c_str();
 
    //短信内容
    char *message = (char*)"您的验证码是：1212。请不要把验证码泄露给其他人。";
 
    /**************** 发送短信 *****************/
    send_sms(account, password, mobile, message);
    printf("send the message is success\n");
    close(basefd);
    exit(0);
    } 
}
/**
 *
 * 邮件类实现
 * */
void SmtpAppeder::log(shared_ptr<Logger>logger, LogLevel::Level le, LogEvent::ptr event){
  toil="系统严重警告";
  mail="服务器出现严重bug,请尽快处理。具体bug情况请参考日志信息：";
	if (le >= level) {
		mail+=formate->format(logger, le, event);
	}
  if(smtp_flag==1){
    Smtp sm(25,smtpIp.c_str(),formail.c_str(),posswd.c_str(),"",toil.c_str(),mail.c_str());
    for(auto it=tomail.begin();it!=tomail.end();it++){
      //添加收件人
      sm.addTargetEmail(it->second.c_str());
    }
   int err;
   if ((err = sm.sendEmail()) != 0)
   {
      if (err == 1)
         cout << "错误1: 由于网络不畅通，发送失败!" << endl;
      if (err == 2)
         cout << "错误2: 用户名错误,请核对!" << endl;
      if (err == 3)
         cout << "错误3: 用户密码错误，请核对!" << endl;
      if (err == 4)
         cout << "错误4: 请检查附件目录是否正确，以及文件是否存在!" << endl;
   }
   

   }  
   
}

/**
 *
 * 日志数据库
 *
 * */
//连接数据库
void MysqlAppender::ConnectMysql(){
    MysqlRall mr(Mysql::GetMysql());//获取数据库对象
    m=mr.GetMysql();
    m->SetIndex(10);
    m->Init();
    //获取连接
     mysql=m->GetConnect(u);
    /**
     *检测数据库中有没有日志表
     * */
    string sql="";
        sql=sql+"SELECT * FROM log_table;";
    //获取查询结果
    m->SelectSql(mysql,sql);
    MYSQL_RES  *res=m->GetAns();
    if(!res){
        cout<<"Jian Biao"<<endl;
        sql="";
        //不存在表
    //没有创建日志表
            sql=sql+"create table log_table(\
				id int(11) primary key not null auto_increment,\
				log_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP(),\
				level varchar(32) not null,\
				log_info TINYTEXT not null)\
				engine=innodb default charset=utf8mb4;";
            m->Insert(mysql,sql);
            cout<<sql<<endl;
       }
}
void MysqlAppender::log(shared_ptr<Logger>logger, LogLevel::Level le, LogEvent::ptr event){
    string log_info="";
	if (le >= level) {
		log_info+=formate->format(logger, le, event);
	}
    string sql="";
    cout<<sql<<endl;
    sql=sql+"insert into log_table(level,log_info)"+"values( \'"+LogLevel::ToString(le)+"\', \'"+
        log_info+"\');";
    cout<<sql<<endl;
    m->Insert(mysql,sql);
    cout<<sql<<endl;

}
/**
 *日志器实现
 **/
void Logger::AddAppender(LogAppender::ptr app) {

	if (!app->GetFormat()) {
		//设置日志格式
		app->SetFormate(formate);
	}
	appenders.push_back(app);
}
void Logger::delAppedner(LogAppender::ptr app) {
	for (auto it = appenders.begin();it != appenders.end();++it) {
		if (*it == app) {
			appenders.erase(it);
			break;
		}
	}
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
	if (level >= this->level) {
		if (!appenders.empty()) {
			for (auto& o : appenders) {
				//将自己作为参数传递过去
				auto self = shared_from_this();
				o->log(self, level, event);//调用日志器所属的输出器，将数据输出到对应的位置
			}
		}
		else {
			root->log(level, event);
		}
	}
}
void Logger::SetFormat(Formater::ptr formate) {
	formate = formate;
	for (auto& o : appenders) {
		o->SetFormate(formate);
	}
}
void Logger::SetStringFormat(string str) {
	Formater::ptr new_val(new Formater(str));
	if (new_val->IsError()) {
		cout << "Logger SetFormater name=" << name <<
			"value=" << formate << "invalid formater" << endl;;
		return;
	}
	SetFormat(new_val);
}

