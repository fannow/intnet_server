#include "smtp.h"
#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

char *base64Encode(char const *origStr, unsigned int origLength)
{
	unsigned char const *orig = (unsigned char const *)origStr; // in case any input bytes have the MSB set
	if (orig == NULL)
		return NULL;

	unsigned const numOrig24BitValues = origLength / 3;
	bool havePadding = origLength > numOrig24BitValues * 3;
	bool havePadding2 = origLength == numOrig24BitValues * 3 + 2;
	unsigned const numResultBytes = 4 * (numOrig24BitValues + havePadding);
	char *result = new char[numResultBytes + 3]; // allow for trailing '/0'

	unsigned i;
	for (i = 0; i < numOrig24BitValues; ++i)
	{
		result[4 * i + 0] = base64Char[(orig[3 * i] >> 2) & 0x3F];
		result[4 * i + 1] = base64Char[(((orig[3 * i] & 0x3) << 4) | (orig[3 * i + 1] >> 4)) & 0x3F];
		result[4 * i + 2] = base64Char[((orig[3 * i + 1] << 2) | (orig[3 * i + 2] >> 6)) & 0x3F];
		result[4 * i + 3] = base64Char[orig[3 * i + 2] & 0x3F];
	}

	if (havePadding)
	{
		result[4 * i + 0] = base64Char[(orig[3 * i] >> 2) & 0x3F];
		if (havePadding2)
		{
			result[4 * i + 1] = base64Char[(((orig[3 * i] & 0x3) << 4) | (orig[3 * i + 1] >> 4)) & 0x3F];
			result[4 * i + 2] = base64Char[(orig[3 * i + 1] << 2) & 0x3F];
		}
		else
		{
			result[4 * i + 1] = base64Char[((orig[3 * i] & 0x3) << 4) & 0x3F];
			result[4 * i + 2] = '=';
		}
		result[4 * i + 3] = '=';
	}

	result[numResultBytes] = '\0';
	return result;
}

char *base64Decode(const char *origStr, unsigned int *resLength)
{
	//根据base64表，以字符找到对应的十进制数据
	int table[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 62, 0, 0, 0,
		       63, 52, 53, 54, 55, 56, 57, 58,
		       59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0,
		       1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
		       13, 14, 15, 16, 17, 18, 19, 20, 21,
		       22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26,
		       27, 28, 29, 30, 31, 32, 33, 34, 35,
		       36, 37, 38, 39, 40, 41, 42, 43, 44,
		       45, 46, 47, 48, 49, 50, 51};

	int len;     //编码后的长度
	int str_len; //解码后的长度
	char *res;   //解码后的字符串
	int i, j;

	//计算解码后的字符串长度
	len = strlen((char *)origStr);
	//判断编码后的字符串后是否有=
	if (strstr((char *)origStr, "=="))
		str_len = len / 4 * 3 - 2;
	else if (strstr((char *)origStr, "="))
		str_len = len / 4 * 3 - 1;
	else
		str_len = len / 4 * 3;

	if (nullptr != resLength)
	{
		*resLength = str_len;
	}

	res = (char *)malloc(sizeof(unsigned char) * str_len + 1);
	res[str_len] = '\0';

	//以4个字符为一位进行解码
	for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
	{
		res[j] = ((unsigned char)table[origStr[i]]) << 2 | (((unsigned char)table[origStr[i + 1]]) >> 4);	    //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合
		res[j + 1] = (((unsigned char)table[origStr[i + 1]]) << 4) | (((unsigned char)table[origStr[i + 2]]) >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合
		res[j + 2] = (((unsigned char)table[origStr[i + 2]]) << 6) | ((unsigned char)table[origStr[i + 3]]);	    //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
	}

	return res;
}

Smtp::Smtp()
{
	this->_content = "";
	this->_port = 25;
	this->_user = "";
	this->_password = "";
	this->_title = "";
	this->_domain = "";

#ifdef _MSC_VER
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
#endif
	this->_sockClient = 0;
}

Smtp::~Smtp()
{
#ifdef _MSC_VER
	closesocket(sockClient);
	WSACleanup();
#else
	close(_sockClient);
#endif
}

Smtp::Smtp(
    int port,
    string srvDomain,
    string userName,
    string password,
    string targetEmail,
    string emailTitle,
    string content)
{
	this->_content = content;
	this->_port = port;
	this->_user = userName;
	this->_password = password;
	this->_targetAddrs.emplace_back(targetEmail);
	this->_title = emailTitle;
	this->_domain = srvDomain;

#ifdef _MSC_VER
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
#endif
	this->_sockClient = 0;
}

bool Smtp::_createConn()
{
	int skClientTemp = (int)socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in saddr;
	hostent *pHostent;
    cout<<"gethostbyname"<<endl;
	pHostent = gethostbyname(_domain.c_str());
    
    cout<<"gethostbyname"<<endl;
	saddr.sin_addr.s_addr = *((unsigned long *)pHostent->h_addr_list[0]);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(_port);
	int err = connect(skClientTemp, (sockaddr *)&saddr, sizeof(saddr));
    cout<<"connext"<<err<<endl;
	if (err != 0)
		return false;
	this->_sockClient = (int)skClientTemp;
	if (false == _recv())
		return false;
	return true;
}

bool Smtp::_send(string &strMessage)
{
	int err = (int)send(_sockClient, strMessage.c_str(), (int)strMessage.length(), 0);

	if (err < 0)
	{
		return false;
	}
	// cout << strMessage.c_str() << endl;
	return true;
}

bool Smtp::_recv()
{   cout<<"recv1"<<endl;
	memset(_buff, 0, sizeof(char) * (MAX_EMAIL_MESSAGE_LEN + 1));
     cout<<"recv2:"<<_sockClient  <<endl;
	int err = recv(_sockClient, _buff, MAX_EMAIL_MESSAGE_LEN, 0);
     cout<<"recv2:"<< err<<endl;
	if (err < 0)
	{
		return false;
	}
	_buff[err] = '\0';
	cout << _buff << endl;
	return true;
}

int Smtp::_login()
{
	string sendBuff;
	sendBuff = "EHLO ";
	sendBuff += _user;
	sendBuff += "\r\n";

	if (false == _send(sendBuff) || false == _recv())
		return 1;

	sendBuff.empty();
	sendBuff = "AUTH LOGIN\r\n";
	if (false == _send(sendBuff) || false == _recv())
		return 1;

	sendBuff.empty();
	int pos = (int)_user.find('@', 0);
	sendBuff = _user.substr(0, pos);

	char *ecode;

	ecode = base64Encode(sendBuff.c_str(), (unsigned int)strlen(sendBuff.c_str()));
	sendBuff.empty();
	sendBuff = ecode;
	sendBuff += "\r\n";
	delete[] ecode;

	if (false == _send(sendBuff) || false == _recv())
		return 1;

	sendBuff.empty();
	ecode = base64Encode(_password.c_str(), (unsigned int)strlen(_password.c_str()));
	sendBuff = ecode;
	sendBuff += "\r\n";
	delete[] ecode;

	if (false == _send(sendBuff) || false == _recv())
		return 1;

	if (NULL != strstr(_buff, "550"))
		return 2;

	if (NULL != strstr(_buff, "535"))
		return 3;
	return 0;
}

bool Smtp::_sendEmailHead()
{
	string sendBuff;
	sendBuff = "MAIL FROM: <" + _user + ">\r\n";
	if (false == _send(sendBuff) || false == _recv())
		return false;

	sendBuff.empty();

	for (string targetAddr : _targetAddrs)
	{
		sendBuff = "RCPT TO: <" + targetAddr + ">\r\n";
		if (false == _send(sendBuff) || false == _recv())
			return false;
	}

	sendBuff.empty();
	sendBuff = "DATA\r\n";
	if (false == _send(sendBuff) || false == _recv())
		return false;

	sendBuff.empty();
	_formatEmailHead(sendBuff);
	if (false == _send(sendBuff))
		return false;

	return true;
}

void Smtp::_formatEmailHead(string &strEmail)
{
	strEmail = "From: ";
	strEmail += _user;
	strEmail += "\r\n";

	strEmail += "To: ";
	if (this->_targetAddrs.size() == 1)
	{
		strEmail += _targetAddrs[0];
	}
	else
	{
		strEmail += "multi-users";
	}
	strEmail += "\r\n";

	strEmail += "Subject: ";
	strEmail += _title;
	strEmail += "\r\n";

	strEmail += "MIME-Version: 1.0";
	strEmail += "\r\n";

	strEmail += "Content-Type: multipart/mixed;boundary=\"qwertyuiop\"";
	strEmail += "\r\n";
	strEmail += "\r\n";
}

bool Smtp::_sendTextBody()
{
	string sendBuff;
	sendBuff = "--qwertyuiop\r\n";
	sendBuff += "Content-Type: text/plain;";
	sendBuff += "charset=\"utf8\"\r\n\r\n";
	sendBuff += _content;
	sendBuff += "\r\n\r\n";
	return _send(sendBuff);
}

bool Smtp::_sendAccessory(const char *fileName)
{
	//文件的大小
	int fileSize;
	//fileName中包含路径名，name中只有文件名
	string name;
	//打开文件
	FILE *fp = fopen(fileName, "r");
	if (fp == nullptr)
		return false;
		
	//获取linux系统下文件大小
	struct stat statbuf;
	stat(fileName, &statbuf);
	fileSize = statbuf.st_size;

	//linux下获取系统文件名
	name = strrchr(fileName, (int)'/') + 1;


	string sendBuff;
	sendBuff = "--qwertyuiop\r\n";
	// sendBuff += "Content-Type: application/octet-stream;\r\n";
	sendBuff += "Content-Type: image/jpeg;\r\n";
	sendBuff += "name=\"";
	sendBuff += name;
	sendBuff += "\"\r\n";
	sendBuff += "Content-Transfer-Encoding: base64\r\n";
	sendBuff += "fileName=\"";
	sendBuff += name;
	sendBuff += "\"\r\n\r\n";

	char *fileContent = (char *)malloc(fileSize);
	fread(fileContent, 1, fileSize, fp);
	char *encode;
	encode = base64Encode(fileContent, fileSize);
	free(fileContent);

	sendBuff += encode;

	delete[] encode;
	//将读入的字符转换为base64编码

	sendBuff += "\r\n\r\n";

	return _send(sendBuff);
}

bool Smtp::_sendEnd()
{
	string sendBuff;
	sendBuff = "--qwertyuiop--";
	sendBuff += "\r\n.\r\n";
	if (false == _send(sendBuff) || false == _recv())
	{
		return false;
	}
	cout << _buff << endl;
	sendBuff.empty();
	sendBuff = "QUIT\r\n";
	return (_send(sendBuff) && _recv());
}

void Smtp::addTargetEmail(string targetEmail)
{
	this->_targetAddrs.emplace_back(targetEmail);
}

void Smtp::addAccessory(string fileName)
{
	this->_accessories.emplace_back(fileName);
}

int Smtp::sendEmail()
{
	if (false == _createConn())
		return 1;
	int err = _login();
	if (err != 0)
		return err;
	if (false == _sendEmailHead())
		return 1;
	if (false == _sendTextBody())
		return 1;
	for (string fileName : this->_accessories)
	{
		if (false == _sendAccessory(fileName.c_str()))
			return 1;
	}
	if (false == _sendEnd())
		return 1;
	return 0;
}

