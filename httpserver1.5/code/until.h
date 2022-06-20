#ifndef __UNTIL_H__
#define __UNTIL_H__
#include<iostream>
#include<string>
#include<sstream>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/syscall.h>
using namespace std;
pid_t GetThreadID() {
    return syscall(SYS_gettid);
}
class Util{
  public:
    static void StringParse(string &line,string &methon,string &uri,string &version){
    stringstream ss(line);
    ss>>methon>>uri>>version;
    }
    static void MakeStringKv(string StringKv,string &k,string &v){
      size_t f =StringKv.find(':');
      if(f!=string::npos){
      k=StringKv.substr(0,f);
      v=StringKv.substr(f+2);

      }
    }
    static int StringToInt(string str){
      return atoi(str.c_str());
    }
};
template<class T, class X = void, int N = 0>
class Signall {
public:
	static T* GetInstance() {

		static T v;
		return &v;
	}
};
#endif
