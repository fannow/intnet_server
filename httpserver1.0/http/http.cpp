#include"http.h"
/**
* 请求方法解析
* 
*/
//解析http请求
void request::Run(int *mepoll) {
    InsertLine(mepoll);
    InsertRqhead(mepoll);
   

    //检查是不是要接收请求体
    if (IsRecvRqBody()) {
        InsertRqBody(mepoll);
    }
    /**
    * http响应分析完毕
    */
    
}
void request::MakeSuff() {
    size_t pos = path.rfind('.');

    if (pos != string::npos) {
        file_type = path.substr(pos + 1);
    }
}
void request::InsertLine(int *mepoll) {
    string line;
    Sock::getline(sock, rq_line, mepoll);
    RqLineParse();//分析请求头
}
void request::RqLineParse() {
    Util::StringParse(rq_line, method, uri, version);//请求行分离
    LOG_INFO(LOG_ROOT()) << rq_line;
    LOG_INFO(LOG_ROOT()) << method;
    LOG_INFO(LOG_ROOT()) << uri;
    LOG_INFO(LOG_ROOT()) << version;
}
/**
* 解析请求头以kv模式插入到map中
* 
*/
void request::RqHeadParse() {
    for (auto& it : rq_head) {
        string k, v;
        Util::MakeStringKv(it, k, v);
        headkv.insert({ k,v });
        if (strcasecmp(k.c_str(), "Content-Length") == 0) {
            rq_body_length = atoi(v.c_str());//保存请求头长度方便后边读取请求体
        }
    }
}
/**
* 循环读取请求头直到读到空行为止
*/
void request::InsertRqhead(int *mepoll) {
    string strhead = "";
    while (1) {
        Sock::getline(sock, strhead, mepoll);
     
        if (strhead.empty()) {
            break;
        }
        else {
            LOG_INFO(LOG_ROOT()) << strhead << endl;
            rq_head.push_back(strhead);
        }
    }
    //读取完直接解析请求头

    RqHeadParse();
}
/**
* 请求路径解析
* 将资源路径和参数分离
*带参数一般要执行cgi将cgi标记设为true
* 剩余的就是资源存放路径
**/
void request::RqUriParse() {
    //存在
    size_t pos = uri.find('?');
    if (pos != string::npos) {
        path += uri.substr(0, pos);
        query_string = uri.substr(pos + 1);
        //get方法uri后边跟着参数
       // 执行cgi
        cgi = true;
    }
    else {
        path += uri;
    }

}
void request::RqUriPost() {
    path += uri;
}
bool request::IsRecvRqBody() {
    //请求方法为post接受并且请求体长度大于0执行cgi程序
    if ((strcasecmp(method.c_str(), "post") == 0) && rq_body_length > 0) {
        //再报头中找Contxt-length--请求体长度不为-1
        //post方法传参参数在请求体中
        //要执行cgi
        cgi = true;
        return true;
    }
    return  false;
}
void request::InsertRqBody(int *mepoll) {
    ssize_t len = rq_body_length;
    char ch;
    string str = "";
    while (len) {
        int s = recv(sock, &ch, 1, 0);
        if (s < 0) {
            LOG_ERROR(LOG_ROOT()) << "recv request error" << endl;
        }
         else if (s == 0) {
            close(sock);
            epoll_ctl(*mepoll, EPOLL_CTL_DEL, sock, NULL);
            cout << "客户端关闭" << endl;
        }
        str += ch;
        len--;
    }
    rq_body = str;
}
/**
* 检查是不是post和get
*/
bool request::Method() {
    if ((strcasecmp(method.c_str(), "post") == 0) || (strcasecmp(method.c_str(), "get") == 0)) {
        return true;
    }
    return false;
    
}

void request::IsInsertIndex() {

    //以/结尾/a/b/结尾都要添加
   //

    if (path[path.size() - 1] == '/') {
        path += HOMEPATH;
    }
}
/**
* 是不是get方法
*/
bool request::IsGet() {
    if ((strcasecmp(method.c_str(), "get") == 0) ) {
        return true;
    }
    return false;
}
/**
* 是不是post方法
*/
bool request::IsPost() {
    if ((strcasecmp(method.c_str(), "post") == 0)) {
        return true;
    }
    return false;
}

/**
* 任务处理累实现
* 
*/

void Make::Run() {
    code = 200;
    //检查是不是post或者get请求不是直接跳过中间解析直接制作响应
    if (!re->Method()) {
       LOG_WARN(LOG_ROOT()) << "request of method error"<<endl;
       goto end;
    }


    //解析uri
    if (re->IsGet()) {
        re->RqUriParse();
    }
    else if(re->IsPost()) {
        re->RqUriPost();
    }
  
 //get方法且query_string有->资源在path，给path传递的参数在query_string中
 //post方式资源在path中传递的参数在请求体中

    //检查path是不是要添加主页
    path = re->GetPath();
    re->IsInsertIndex();


    //检查path是不是有效路径是不是在webroot更目录下
    struct stat st;
    //检测path有效返回st.st_mode的的文件对应的模式，文件或目录
    if (stat(path.c_str(), &st) < 0) {
        //文件不存在路径

        LOG_WARN(LOG_ROOT()) << "path is html 404" << endl;;
        //      Log(WARNIG,"path is not html 404");
        code = 404;
    }
    else {
        //路径存在
        if (S_ISDIR(st.st_mode)) {
            //检测返回的是不是路经
            LOG_DEBUG(LOG_ROOT()) << "path bug " << endl;
            //????存在bugg
            //是以/a/b结尾的但是b是一个目录
            path += "/";
            LOG_DEBUG(LOG_ROOT()) << "path:" << path << endl;
            re->SetPath(path);
            LOG_DEBUG(LOG_ROOT()) << "path:" << re->GetPath() << endl;
            re->IsInsertIndex();
            //设置为/a/b/index.html 
        }
        else {
            if (S_IXUSR & st.st_mode || st.st_mode & S_IXOTH || st.st_mode & S_IXGRP) {
                //检查是不是可执行文件
                  //执行文件返回
                  //cgi处理
                re->SetCgi();
            }
            else {
                //正常的html文件
            }

        }

        if (!re->GetCgi()) {
            //不需要返回结果
            file_size = st.st_size;//普通文件直接设置文件大小

        }
    }
end:
    //制作响应行
    MakeRoLineParse();

    //制作响应头
    MakeRoHead();
}
string Make::MakeSuff(string suf) {
    string su = "text/html";
    if (suff == "css") {
        su = "application/x-csi";
    }
    else if (suff == "html") {
        su = "text/html";
    }
    else  if (suff == "jpg") {
        su = "image/jpeg";
    }
    else if (suff == "jpe") {
        su = "image/jpeg";
    }
    else if (suff == "js") {
        su = "application/x-jpg";
    }

    return su;

}
void Make::MakeRoLineParse() {
    MakeRoLine();
}
void Make::MakeRoLine() {
    ro_line += VERSION;
    ro_line += " ";
    ro_line += to_string(code);
    ro_line += " ";
    ro_line += CodetoStatus();//code转换为状态码
    ro_line += "\r\n";
}
string Make::CodetoStatus() {
    string status = "";
    switch (code) {
    case 400:
        status = "fork you";
        break;
    case 404:
        status = "NOT FOUND";
        break;
    case 200:
        status = "OK";
        break;
    default:
        status = "OK";
        break;

    }
    return status;
}
/**
* 响应头制作
*/
void Make::MakeRoHead() {
    re.MakeSuff();
    string suff = re.GetSuff();
    string suff_ans = MakeSuff(suff);
    string suffparse = "Content-Type: ";
    suffparse += suff_ans;
    suffparse += "\r\n";
    LOG_DEBUG(LOG_ROOT()) << "repose head" << suffparse<<endl;
    ro_head.push_back(suffparse);
}
/**
* 响应函数实现
*/
void response::Run() {
    resp_line = make->GetRoLine();
    resp_head = make->GetRoHead();
    resp_head.push_back(resp_space);

    //发送响应行
    int s = send(sock, resp_line.c_str(), strlen(resp_line.c_str()), 0);
    if (s < 0) {
        LOG_ERROR(LOG_ROOT()) << "resp_line send error" << endl;
    }
    for (auto& it : resp_head) {
        int c = send(sock, it.c_str(), strlen(it.c_str()), 0);
        if (s < 0) {
            LOG_ERROR(LOG_ROOT()) << "resp_head send error" << endl;
        }
    }
    //检查是不是有可执行文件或者直接发送请求资源
    if (re->GetCgi()) {
        //执行可执行文件
        Cgi();
    }
    else {
        //请求没有参数
      //不用返回结果
      //直接返回请求资源
        SendFile();
    }
}
void response::Cgi() {

    int pip_in[2] = { 0 };
    int pip_out[2] = { 0 };
    pipe(pip_in);
    pipe(pip_out);
    string path = re->GetPath();
    string method = re->GetMethod();
    string method_env = "METHOD=";//通过环境变量来
    method_env += method;
    string query_string;
    string query_env;
    string content_length_env;
    string body;
    putenv((char*)method_env.c_str());//将method_env请求方式导入到系统环境变量，方便子进程获取
    pid_t tid = fork();

    if (tid == 0) {
        //       子进程通过只写不读
        close(pip_in[1]);
        close(pip_out[0]);
        dup2(pip_in[0], 0);//重定向到stdin读取
        dup2(pip_out[1], 1);//重定向到stdout写入
        //传递请求方法到子进程
          //一种是通过环境变量给子进程  ->GET方法由父进程给子进程 ->uri的请求参数query_string 因为数量有限
        if (re->IsGet()) {
            query_string = re->Getquery();
            query_env = "QUERY_STRING=";//通过环境变量来
            query_env += query_string;
            putenv((char*)query_env.c_str());//将请求参数导入的系统环境变量中

        }
        else if (re->IsPost()) {
            content_length_env = "CONTENT-LENGTH=";
            content_length_env += to_string(re->GetContextLength());

            putenv((char*)content_length_env.c_str());
        }
        else {

        }
        //执行程序将结果返回给父进程
        //程序替换
        execl(path.c_str(), path.c_str(), NULL);
        exit(0);
    }
    close(pip_in[0]);
    close(pip_out[1]);
    //另一种是通过管道传递给子进程 ->POST由父进程传递给子进程 ->  请求体
    if (re->IsPost()) {

        body = re->GetRqBody();
        //    ssize_t size=body.size();
        //    while(size){
        //      write(pip_in[1],&c,1);
         //     size--;
          //  }
        char ch = 'X';

        for (size_t i = 0;i < body.size();i++) {
            ch = body[i];
            write(pip_in[1], &ch, 1);

        }
    }
    char ch = 0;
    ssize_t s = 0;
    do {
        s = read(pip_out[0], &ch, 1);//读取子进程返回的结果
        if (s > 0) {
            send(sock, &ch, 1, 0);//直接发送子进程返回的结果
        }
    } while (s > 0);
    //进程等待
    waitpid(tid, NULL, 0);
    //close(pip_in[1]);
   // close(pip_out[0]);

}
void response::SendFiles() {
    LOG_INFO(LOG_ROOT()) << "file is sending";
       //先打开文件
    string path =re->GetPath();
    int fd = open(path.c_str(), O_RDONLY);//只读方式打开文件

    if (fd < 0) {
        LOG_ERROR(LOG_ROOT()) << "open file is bug";
        return;
    }
    /**
    * 最内核之间传递数据避免了内核缓冲区和用户缓冲区之间的数据拷贝，效率很高，被称为零拷贝。
    */
    sendfile(sock, fd, NULL, make->GetFileSize());


    close(fd);
}