#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include <sys/socket.h>
#include "netdb.h"

#define DBG_TAG "udp_demo"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#include "mlog.h"

void udp_thread_entry(void *parameter)
{
    int ret;
    /* 创建一个socket，协议簇为AT Socket 协议栈，类型是SOCK_DGRAM，UDP类型 */
    int sock_fd = socket(AF_INET,SOCK_DGRAM,0);
    if (sock_fd  == -1)
    {
        rt_kprintf("Socket error\n");
        return;
    }

    // 2、绑定本地的相关信息，如果不绑定，则系统会随机分配一个端口号
    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;                                //使用IPv4地址
    local_addr.sin_addr.s_addr = inet_addr("192.168.8.127");        //本机IP地址
    local_addr.sin_port = htons(12369);                             //端口
    bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr));//将套接字和IP、端口绑定

    /* 通过函数参数获得host地址（如果是域名，会做域名解析） */
    struct hostent *host;
    host = (struct hostent *) gethostbyname("192.168.8.100");

    /* 初始化预连接的服务端地址 */
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12396);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    ret = sendto(sock_fd, "Hello world!", sizeof("Hello world!"), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret > 0 ) {
        char buf[9] = {'u', 'd', 'p', '_','o', 'k', ' ', ' ', '\0'};
        addNewLineLn(buf);
        rt_kprintf("send to ok\r\n");
    } else {
        rt_kprintf("send to err \r\n");
    }
    while (1)
    {
        /* 等待接收对方发送的数据 */
        struct sockaddr_in recv_addr;
        socklen_t addrlen = sizeof(recv_addr);
        char recvbuf[1024] = {0};
        recvfrom(sock_fd, recvbuf, 1024, 0,(struct sockaddr*)&recv_addr,&addrlen);  //1024表示本次接收的最大字节数

        rt_kprintf("recv :%s \n",recvbuf);
        addNewLineLn(recvbuf);
    }
	/* 关闭这个socket */
    closesocket(sock_fd);
}

int udp_demo(void) {
    char buf[9] = {'u', 'd', 'p', '_','d', 'e', 'm', 'o', '\0'};
    addNewLineLn(buf);
    rt_thread_t udpDemoThread;
    udpDemoThread = rt_thread_create("udpdemo", udp_thread_entry, RT_NULL, 4096, 10, 20);
    if (udpDemoThread == RT_NULL) {
        LOG_D("create udp demo thread err");
        return -RT_ENOMEM;
    }
    rt_thread_startup(udpDemoThread);

    return RT_EOK;
}

MSH_CMD_EXPORT(udp_demo, udp_demo);
