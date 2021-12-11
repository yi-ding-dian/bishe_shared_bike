#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<event.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<event2/util.h>

#include "bike.pb.h"
#include <string>

typedef unsigned short u16;
typedef unsigned int   u32;     /* int == long */
typedef signed char    i8;
typedef signed short   i16;
typedef signed int     i32;     /* int == long */

using namespace std;
using namespace tutorial;

i32 icode = 0;

int tcp_connect_server(const char* server_ip, int port);


void cmd_msg_cb(int fd, short events, void* arg);
void server_msg_cb(struct bufferevent* bev, void* arg);
void event_cb(struct bufferevent* bev, short event, void* arg);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("please input 2 parameter\n");
        return -1;
    }


    //两个参数依次是服务器端的IP地址、端口号
    int sockfd = tcp_connect_server(argv[1], atoi(argv[2]));
    if (sockfd == -1)
    {
        perror("tcp_connect error ");
        return -1;
    }

    printf("connect to server successful\n");

    struct event_base* base = event_base_new();

    struct bufferevent* bev = bufferevent_socket_new(base, sockfd,
        BEV_OPT_CLOSE_ON_FREE);

    //监听终端输入事件
    struct event* ev_cmd = event_new(base, STDIN_FILENO,
        EV_READ | EV_PERSIST, cmd_msg_cb,
        (void*)bev);
    event_add(ev_cmd, NULL);

    //当socket关闭时会用到回调参数
    bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void*)ev_cmd);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);


    event_base_dispatch(base);

    printf("finished \n");
    return 0;
}






void cmd_msg_cb(int fd, short events, void* arg)
{
    char cmd[1024];
    char msg[1024];
    string proto_msg;

    int ret = read(fd, cmd, sizeof(cmd));
    if (ret < 0)
    {
        perror("read fail ");
        exit(1);
    }
    //最后输入的msg 中含有\n 符
    cmd[ret - 1] = '\0';
    printf("read msg: [%s]\n", cmd);

    struct bufferevent* bev = (struct bufferevent*)arg;
    if (strcmp(cmd, "send_mobr") == 0)
    {
        mobile_request  mr;
        mr.set_mobile("18684518289");

        int len = mr.ByteSizeLong();
        memcpy(msg, "FBEB", 4);

        *(u16*)(msg + 4) = 1;//EEVENTID_GET_MOBLIE_CODE_REQ
        printf("*(u16*)(msg + 4)=%d\n", *(u16*)(msg + 4));
        *(i32*)(msg + 6) = len;
        printf("*(i32*)(msg + 6)=%d\n", *(i32*)(msg + 6));
        mr.SerializeToArray(msg + 10, len);

        //发送头部
        bufferevent_write(bev, msg, len + 10);
    }
    if (strcmp(cmd, "send_logr") == 0)
    {
        login_request  lr;
        lr.set_mobile("18684518289");
        lr.set_icode(icode);

        int len = lr.ByteSizeLong();
        memcpy(msg, "FBEB", 4);

        *(u16*)(msg + 4) = 3;//EEVENTID_GET_MOBLIE_CODE_REQ
        printf("*(u16*)(msg + 4)=%d\n", *(u16*)(msg + 4));
        *(i32*)(msg + 6) = len;
        printf("*(i32*)(msg + 6)=%d\n", *(i32*)(msg + 6));
        printf("lr.ByteSizeLong()=%d\n", lr.ByteSizeLong());
        lr.SerializeToArray(msg + 10, len);

        //发送头部
        bufferevent_write(bev, msg, len + 10);
    }
}


void server_msg_cb(struct bufferevent* bev, void* arg)
{
    char msg[1024];

    int len = bufferevent_read(bev, msg, sizeof(msg));
    msg[len] = '\0';

    printf("recv %s from server, len: %d\n", msg, len);

    if (strncmp(msg, "FBEB", 4) == 0)
    {
        mobile_response mr;
        u16 code = *(u16*)(msg + 4);
        i32 len = *(i32*)(msg + 6);

        if (code == 2)
        {
            mobile_response mr;
            mr.ParseFromArray(msg + 10, len);
            icode = mr.icode();
            printf("mobile_response: code:%d, icode:%d, data:%s\n", mr.code(), mr.icode(), mr.data().c_str());
        }
        else if (code == 4)
        {
            login_response lr;
            lr.ParseFromArray(msg + 10, len);
            icode = lr.code();
            printf("login_response: code:%d, data:%s\n", lr.code(), lr.desc().c_str());
        }



    }

}


void event_cb(struct bufferevent* bev, short event, void* arg)
{

    if (event & BEV_EVENT_EOF)
        printf("connection closed\n");
    else if (event & BEV_EVENT_ERROR)
        printf("some other error\n");

    //这将自动close套接字和free读写缓冲区
    bufferevent_free(bev);

    struct event* ev = (struct event*)arg;
    //因为socket已经没有，所以这个event也没有存在的必要了
    event_free(ev);
}


typedef struct sockaddr SA;
int tcp_connect_server(const char* server_ip, int port)
{
    int sockfd, status, save_errno;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    status = inet_aton(server_ip, &server_addr.sin_addr);

    if (status == 0) //the server_ip is not valid value
    {
        errno = EINVAL;
        return -1;
    }

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return sockfd;


    status = connect(sockfd, (SA*)&server_addr, sizeof(server_addr));

    if (status == -1)
    {
        save_errno = errno;
        close(sockfd);
        errno = save_errno; //the close may be error
        return -1;
    }

    evutil_make_socket_nonblocking(sockfd);

    return sockfd;
}
