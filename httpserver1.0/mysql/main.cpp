#include"Mysql.h"
int main(){
    MysqlRall mr((Mysql::GetMysql()));
    Mysql *m=mr.GetMysql();
    m->Init(); 
    User*u=new User("127.0.0.1","root","254819li","db",3306);
     MYSQL *mysql=m->GetConnect(u);
     string sql="insert into st(id, name) values (101,\'李四\')";
     string sql2="select * form st";


      m->Insert(mysql,sql);
}
