#ifndef __SMTP_H__
#define __SMTP_H__

#include <unistd.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

const int MAX_EMAIL_MESSAGE_LEN = 4096;

static const char base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *base64Encode(char const *origStr, unsigned int origLength);
char *base64Decode(char const *origStr, unsigned int *resLength);

class Smtp
{
public:
	Smtp();
	Smtp(
	    int port,
	    string srvDomain,	//smtp服务器域名
	    string userName,	//用户名
	    string password,	//密码
	    string targetEmail, //目的邮件地址
	    string emailTitle,	//主题
	    string content	//内容
	);
	~Smtp();

	void setSrvDomain(string &strDomain) { this->_domain = strDomain; }
	void setUserName(string &strUser) { this->_user = strUser; }
	void setPass(string &strPass) { this->_password = strPass; }
	void setEmailTitle(string &strTitle) { this->_title = strTitle; }
	void setContent(string &strContent) { this->_content = strContent; }
	void setPort(int nPort) { this->_port = nPort; }

	void addTargetEmail(string targetEmail);
	void addAccessory(string fileName);
	int sendEmail();

private:
	int _port;
	string _domain;
	string _user;
	string _password;
	vector<string> _targetAddrs;
	string _title;
	//邮件正文
	string _content;
	//附件文件名包含路径
	vector<string> _accessories;
	char _buff[MAX_EMAIL_MESSAGE_LEN + 1];
	int _buffLen;
	int _sockClient;

	bool _createConn();
	bool _send(string &strMessage);
	bool _recv();
	void _formatEmailHead(string &strEmail);
	int _login();
	bool _sendEmailHead();
	bool _sendTextBody();
	bool _sendAccessory(const char *fileName);
	bool _sendEnd();
};

#endif // !__SMTP_H__

