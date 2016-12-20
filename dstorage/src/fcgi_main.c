#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fcgi_stdio.h"

int main(int argc, char *argv[])
{
    int count = 0;

    //阻塞等待ngxin服务器 给传递数据
    while (FCGI_Accept() >= 0) {

        //自定义业务
        //cgi程序会将 stdout重定向到 nginx->已经accept成功的cfd上
        //          将stdin 重定向到  nginx已经accept成功的cfd上
        printf("Content-type: text/html\r\n");
        printf("\r\n");
        printf("<title>Fast CGI Hello!</title>");
        printf("<h1>Fast CGI Hello!</h1>");
        printf("Request number %d running on host <i>%s</i>\n", ++count, getenv("SERVER_NAME"));
        printf("query_string: %s\r\n", getenv("QUERY_STRING"));
        printf("remote addr: %s\r\n", getenv("REMOTE_ADDR"));
        printf("remote port: %s\r\n", getenv("REMOTE_PORT"));
        printf("server addr: %s\r\n", getenv("SERVER_ADDR"));
        printf("server port: %s\r\n", getenv("SERVER_PORT"));

        //redis
        //fastDFs
    }

    return 0;
}
