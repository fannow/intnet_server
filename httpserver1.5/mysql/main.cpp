#include"Mysql.h"
int main(){
    MysqlRall mr((Mysql::GetMysql()));
    Mysql *m=mr.GetMysql();
    m->Init(); 
    int sum=50;
    while(sum--){
    User*u=new User("127.0.0.1","root","254819li","db",3306);
     MYSQL *mysql=m->GetConnect(u);
     string sql="insert into st(id, name) values (";
     sql=sql+to_string(sum)+",\'lis\');";
    cout<<sql<<endl;

      m->Insert(mysql,sql);
      

      }     
}
