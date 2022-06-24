#include"../code/httpserver.h"
#include"../code/Log.h"
static void User(string use){
  cout<<"user:"<<use<<endl;

}
int main(int argc,char *argv[]){
  if(argc!=2){
    User(argv[0]);
    return 1;
  }
 // daemon(1,1);
//HttpServer *hs=new HttpServer(atoi(argv[1]));
HttpServer *hs=HttpServer::GetInstance(atoi(argv[1]));
hs->Init();
LOG_INFO(LOG_ROOT())<<"The server started successfully!";
hs->Start();
}
