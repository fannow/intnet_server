#include"http.h"
/**
* ���󷽷�����
* 
*/
//����http����
void request::Run(int *mepoll) {
    InsertLine(mepoll);
    InsertRqhead(mepoll);
   

    //����ǲ���Ҫ����������
    if (IsRecvRqBody()) {
        InsertRqBody(mepoll);
    }
    /**
    * http��Ӧ�������
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
    RqLineParse();//��������ͷ
}
void request::RqLineParse() {
    Util::StringParse(rq_line, method, uri, version);//�����з���
    LOG_INFO(LOG_ROOT()) << rq_line;
    LOG_INFO(LOG_ROOT()) << method;
    LOG_INFO(LOG_ROOT()) << uri;
    LOG_INFO(LOG_ROOT()) << version;
}
/**
* ��������ͷ��kvģʽ���뵽map��
* 
*/
void request::RqHeadParse() {
    for (auto& it : rq_head) {
        string k, v;
        Util::MakeStringKv(it, k, v);
        headkv.insert({ k,v });
        if (strcasecmp(k.c_str(), "Content-Length") == 0) {
            rq_body_length = atoi(v.c_str());//��������ͷ���ȷ����߶�ȡ������
        }
    }
}
/**
* ѭ����ȡ����ͷֱ����������Ϊֹ
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
    //��ȡ��ֱ�ӽ�������ͷ

    RqHeadParse();
}
/**
* ����·������
* ����Դ·���Ͳ�������
*������һ��Ҫִ��cgi��cgi�����Ϊtrue
* ʣ��ľ�����Դ���·��
**/
void request::RqUriParse() {
    //����
    size_t pos = uri.find('?');
    if (pos != string::npos) {
        path += uri.substr(0, pos);
        query_string = uri.substr(pos + 1);
        //get����uri��߸��Ų���
       // ִ��cgi
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
    //���󷽷�Ϊpost���ܲ��������峤�ȴ���0ִ��cgi����
    if ((strcasecmp(method.c_str(), "post") == 0) && rq_body_length > 0) {
        //�ٱ�ͷ����Contxt-length--�����峤�Ȳ�Ϊ-1
        //post�������β�������������
        //Ҫִ��cgi
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
            cout << "�ͻ��˹ر�" << endl;
        }
        str += ch;
        len--;
    }
    rq_body = str;
}
/**
* ����ǲ���post��get
*/
bool request::Method() {
    if ((strcasecmp(method.c_str(), "post") == 0) || (strcasecmp(method.c_str(), "get") == 0)) {
        return true;
    }
    return false;
    
}

void request::IsInsertIndex() {

    //��/��β/a/b/��β��Ҫ���
   //

    if (path[path.size() - 1] == '/') {
        path += HOMEPATH;
    }
}
/**
* �ǲ���get����
*/
bool request::IsGet() {
    if ((strcasecmp(method.c_str(), "get") == 0) ) {
        return true;
    }
    return false;
}
/**
* �ǲ���post����
*/
bool request::IsPost() {
    if ((strcasecmp(method.c_str(), "post") == 0)) {
        return true;
    }
    return false;
}

/**
* ��������ʵ��
* 
*/

void Make::Run() {
    code = 200;
    //����ǲ���post����get������ֱ�������м����ֱ��������Ӧ
    if (!re->Method()) {
       LOG_WARN(LOG_ROOT()) << "request of method error"<<endl;
       goto end;
    }


    //����uri
    if (re->IsGet()) {
        re->RqUriParse();
    }
    else if(re->IsPost()) {
        re->RqUriPost();
    }
  
 //get������query_string��->��Դ��path����path���ݵĲ�����query_string��
 //post��ʽ��Դ��path�д��ݵĲ�������������

    //���path�ǲ���Ҫ�����ҳ
    path = re->GetPath();
    re->IsInsertIndex();


    //���path�ǲ�����Ч·���ǲ�����webroot��Ŀ¼��
    struct stat st;
    //���path��Ч����st.st_mode�ĵ��ļ���Ӧ��ģʽ���ļ���Ŀ¼
    if (stat(path.c_str(), &st) < 0) {
        //�ļ�������·��

        LOG_WARN(LOG_ROOT()) << "path is html 404" << endl;;
        //      Log(WARNIG,"path is not html 404");
        code = 404;
    }
    else {
        //·������
        if (S_ISDIR(st.st_mode)) {
            //��ⷵ�ص��ǲ���·��
            LOG_DEBUG(LOG_ROOT()) << "path bug " << endl;
            //????����bugg
            //����/a/b��β�ĵ���b��һ��Ŀ¼
            path += "/";
            LOG_DEBUG(LOG_ROOT()) << "path:" << path << endl;
            re->SetPath(path);
            LOG_DEBUG(LOG_ROOT()) << "path:" << re->GetPath() << endl;
            re->IsInsertIndex();
            //����Ϊ/a/b/index.html 
        }
        else {
            if (S_IXUSR & st.st_mode || st.st_mode & S_IXOTH || st.st_mode & S_IXGRP) {
                //����ǲ��ǿ�ִ���ļ�
                  //ִ���ļ�����
                  //cgi����
                re->SetCgi();
            }
            else {
                //������html�ļ�
            }

        }

        if (!re->GetCgi()) {
            //����Ҫ���ؽ��
            file_size = st.st_size;//��ͨ�ļ�ֱ�������ļ���С

        }
    }
end:
    //������Ӧ��
    MakeRoLineParse();

    //������Ӧͷ
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
    ro_line += CodetoStatus();//codeת��Ϊ״̬��
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
* ��Ӧͷ����
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
* ��Ӧ����ʵ��
*/
void response::Run() {
    resp_line = make->GetRoLine();
    resp_head = make->GetRoHead();
    resp_head.push_back(resp_space);

    //������Ӧ��
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
    //����ǲ����п�ִ���ļ�����ֱ�ӷ���������Դ
    if (re->GetCgi()) {
        //ִ�п�ִ���ļ�
        Cgi();
    }
    else {
        //����û�в���
      //���÷��ؽ��
      //ֱ�ӷ���������Դ
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
    string method_env = "METHOD=";//ͨ������������
    method_env += method;
    string query_string;
    string query_env;
    string content_length_env;
    string body;
    putenv((char*)method_env.c_str());//��method_env����ʽ���뵽ϵͳ���������������ӽ��̻�ȡ
    pid_t tid = fork();

    if (tid == 0) {
        //       �ӽ���ͨ��ֻд����
        close(pip_in[1]);
        close(pip_out[0]);
        dup2(pip_in[0], 0);//�ض���stdin��ȡ
        dup2(pip_out[1], 1);//�ض���stdoutд��
        //�������󷽷����ӽ���
          //һ����ͨ�������������ӽ���  ->GET�����ɸ����̸��ӽ��� ->uri���������query_string ��Ϊ��������
        if (re->IsGet()) {
            query_string = re->Getquery();
            query_env = "QUERY_STRING=";//ͨ������������
            query_env += query_string;
            putenv((char*)query_env.c_str());//��������������ϵͳ����������

        }
        else if (re->IsPost()) {
            content_length_env = "CONTENT-LENGTH=";
            content_length_env += to_string(re->GetContextLength());

            putenv((char*)content_length_env.c_str());
        }
        else {

        }
        //ִ�г��򽫽�����ظ�������
        //�����滻
        execl(path.c_str(), path.c_str(), NULL);
        exit(0);
    }
    close(pip_in[0]);
    close(pip_out[1]);
    //��һ����ͨ���ܵ����ݸ��ӽ��� ->POST�ɸ����̴��ݸ��ӽ��� ->  ������
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
        s = read(pip_out[0], &ch, 1);//��ȡ�ӽ��̷��صĽ��
        if (s > 0) {
            send(sock, &ch, 1, 0);//ֱ�ӷ����ӽ��̷��صĽ��
        }
    } while (s > 0);
    //���̵ȴ�
    waitpid(tid, NULL, 0);
    //close(pip_in[1]);
   // close(pip_out[0]);

}
void response::SendFiles() {
    LOG_INFO(LOG_ROOT()) << "file is sending";
       //�ȴ��ļ�
    string path =re->GetPath();
    int fd = open(path.c_str(), O_RDONLY);//ֻ����ʽ���ļ�

    if (fd < 0) {
        LOG_ERROR(LOG_ROOT()) << "open file is bug";
        return;
    }
    /**
    * ���ں�֮�䴫�����ݱ������ں˻��������û�������֮������ݿ�����Ч�ʺܸߣ�����Ϊ�㿽����
    */
    sendfile(sock, fd, NULL, make->GetFileSize());


    close(fd);
}