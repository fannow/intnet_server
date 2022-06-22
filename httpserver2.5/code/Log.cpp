#include"Log.h"
/*
 *ʵ����־���¼�
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
 * ��־�¼�ʵ��
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
��־���Ƶ�Ԫʵ��
**/

//��Ϣ���Ƶ�Ԫ

class Messageitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	Messageitemforma(string str = "") {}

	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->getSS();
	}

};
//��־������Ƶ�Ԫ
class Levelitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	Levelitemforma(string str = "") {}

	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << LogLevel::ToString(level);
	}

};
//��������������ʱ��
class Pleaseitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	Pleaseitemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetTime();
	}

};
//��־������
class lognameitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	lognameitemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetLogger()->GetLoggerName();
	}
};


//�߳�id
class ThreadIditemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	ThreadIditemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
		os << event->GetThreadId();
	}
};
//Э��id
class fiberiditemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	fiberiditemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetFiberId();
	}
};

//����ʱ��
class Timeitemforma :public Formater::Item {
private:
	//ʱ���ʽ
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
		//��־ʱ���ʽ����
		struct tm* tm;
		time_t t = event->GetNowTime();
		tm = localtime(&t);
		char buff[64];
		//ʱ���ʽ���ú���
		strftime(buff, sizeof(buff), time_formate.c_str(), tm);
		os << buff;
	}

};

//�ļ�����
class filenameitemforma :public Formater::Item {
public:
	typedef shared_ptr<Messageitemforma> ptr;
	filenameitemforma(string str = "") {}
	void format(ostream& os, shared_ptr<Logger> logger, LogLevel::Level leve, LogEvent::ptr event) override {
		os << event->GetName();
	}
};
//�ļ��к�
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
��־��ʽ����ʵ��

//*///string Formater::format(shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr evnt) {
//	stringstream  ss;
//	int i = 0;
//	for (int i = 0;i < m_items.size();i++) {
//		cout << "i" << endl;
//		//������־��ʽ���Ƶ�Ԫ��formdar������д��ss ���ظ���־�����������־�����
//		m_items[i]->format(ss, logger, level, evnt);
//	}
//	return ss.str();
//}
//����־��ʽ����־��ʽ���Ƶ�Ԫ��Ӧ�������뵽vector��
void Formater::Init() {
	//tuple(str,fmt,type)
	vector<tuple<string, string, int>>vec;

	string nstr;
	for (size_t i = 0;i < formate.size();i++) {
		if (formate[i] != '%') {
			nstr.append(1, formate[i]);
			continue;
		}
		//��ʵ����һ��%
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
			//�����ո�
			if (!isalpha(formate[n]) && formate[n] != '}' && formate[n] != '{') {
				str = formate.substr(i + 1, n - i - 1);
				break;
			}
			if (fmt_staus == 0) {
				if (formate[n] == '{') {
					//�ַ����ָ�
					str = formate.substr(i + 1, n - i - 1);
					fmt_staus = 1;
					fmt_beign = n;
					n++;
					continue;
				}

			}
			else if (fmt_staus == 1) {
				if (formate[n] == '}') {
					//�ַ����ָ�
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

			//������־����־�洢��������

			vec.push_back(make_tuple(str, fmt, 1));
			i = n - 1;
		}
		else if (fmt_staus == 1) {
			isno = true;
			cout << "pattern parse error" << formate << "-" << formate.substr(i) << endl;
			//��ʽ����
			vec.push_back(make_tuple("<<pattern_error>>", fmt, 0));
		}
	}
	if (!nstr.empty()) {
		vec.push_back(make_tuple(nstr, "", 0));

	}

	/*
	* %m--->��Ϣ��
	* %p--->��־����
	* %r--->������ʱ��
	* %c--->��־����
	* %t--->�߳�id
	* %n--->�س�
	* %d--->ʱ��
	* %f--->�ļ���
	* %l--->�к�
	*/

	//function<Item::ptr(string str)> ����ֵΪiter::ptr ����Ϊstring�ú���
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
 *�ļ������ʵ��
 *
 * */
void FileAppender::log(shared_ptr<Logger> logger, LogLevel::Level le, LogEvent::ptr event) {
	//������д����־�ļ�
	//�Ȳ����ļ��ǲ��Ǵ�
	//Ҫ���Ǵ�״̬ ���´��ļ�
   // ����д����־�ļ�
	if (le >= level) {
		file_stream << formate->format(logger, le, event);
	}

}

bool FileAppender::reopen() {
	if (!file_stream) {
		//�ȹر��ļ������´�
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
//���Ӷ��Ž�����
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
* ��http post����
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
* ���Ͷ���
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
 //���ŷ���
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
    //�鿴�û��� ��¼�û�����->��֤��֪ͨ����>��Ʒ����->API�ӿ���Ϣ->APIID
     char *account = (char*)"C84353732";
 
    //�鿴���� ��¼�û�����->��֤��֪ͨ����>��Ʒ����->API�ӿ���Ϣ->APIKEY
    char *password =(char*) "28e86569d167048485fdb898254db7da";
 
    //�ֻ���
    char *mobile = (char*)it->second.c_str();
 
    //��������
    char *message = (char*)"������֤���ǣ�1212���벻Ҫ����֤��й¶�������ˡ�";
 
    /**************** ���Ͷ��� *****************/
    send_sms(account, password, mobile, message);
    printf("send the message is success\n");
    close(basefd);
    exit(0);
    } 
}
/**
 *
 * �ʼ���ʵ��
 * */
void SmtpAppeder::log(shared_ptr<Logger>logger, LogLevel::Level le, LogEvent::ptr event){
  toil="ϵͳ���ؾ���";
  mail="��������������bug,�뾡�촦������bug�����ο���־��Ϣ��";
	if (le >= level) {
		mail+=formate->format(logger, le, event);
	}
  if(smtp_flag==1){
    Smtp sm(25,smtpIp.c_str(),formail.c_str(),posswd.c_str(),"",toil.c_str(),mail.c_str());
    for(auto it=tomail.begin();it!=tomail.end();it++){
      //����ռ���
      sm.addTargetEmail(it->second.c_str());
    }
   int err;
   if ((err = sm.sendEmail()) != 0)
   {
      if (err == 1)
         cout << "����1: �������粻��ͨ������ʧ��!" << endl;
      if (err == 2)
         cout << "����2: �û�������,��˶�!" << endl;
      if (err == 3)
         cout << "����3: �û����������˶�!" << endl;
      if (err == 4)
         cout << "����4: ���鸽��Ŀ¼�Ƿ���ȷ���Լ��ļ��Ƿ����!" << endl;
   }
   

   }  
   
}

/**
 *
 * ��־���ݿ�
 *
 * */
//�������ݿ�
void MysqlAppender::ConnectMysql(){
    MysqlRall mr(Mysql::GetMysql());//��ȡ���ݿ����
    m=mr.GetMysql();
    m->SetIndex(10);
    m->Init();
    //��ȡ����
     mysql=m->GetConnect(u);
    /**
     *������ݿ�����û����־��
     * */
    string sql="";
        sql=sql+"SELECT * FROM log_table;";
    //��ȡ��ѯ���
    m->SelectSql(mysql,sql);
    MYSQL_RES  *res=m->GetAns();
    if(!res){
        cout<<"Jian Biao"<<endl;
        sql="";
        //�����ڱ�
    //û�д�����־��
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
 *��־��ʵ��
 **/
void Logger::AddAppender(LogAppender::ptr app) {

	if (!app->GetFormat()) {
		//������־��ʽ
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
				//���Լ���Ϊ�������ݹ�ȥ
				auto self = shared_from_this();
				o->log(self, level, event);//������־����������������������������Ӧ��λ��
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

