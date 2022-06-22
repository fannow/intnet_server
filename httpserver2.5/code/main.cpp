#include "q.hpp"
#include <iostream>
using namespace std;

int main()
{
   Smtp smtp(
       465,                                                                    /*smtp端口*/
       "smtp.qq.com",                                                        /*smtp服务器地址*/
       "185334690@qq.com",                                                 /*你的邮箱地址*/
       "ysidtuhpbiwxbhdg",                                                           /*邮箱密码*/
       "1702959373@qq.com",                                              		/*目的邮箱地址*/
       "这是邮件标题",                                                              /*主题*/
       "这是邮件正文." 														/*邮件正文*/
   );

    //添加第二个收件人
   smtp.addTargetEmail("wsad@qq.com");
   smtp.addAccessory("/home/bins/1.png");
   smtp.addAccessory("/home/bins/2.png");
   smtp.addAccessory("/home/bins/3.png");
   smtp.addAccessory("/home/bins/4.png");
   smtp.addAccessory("/home/bins/5.png");

   int err;
   if ((err = smtp.sendEmail()) != 0)
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
   return 0;
}

