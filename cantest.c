#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/if.h>

int run = 1;

void exit_fun(void)
{
    run = 0;
}

int can_recv(char *can_dev)
{
    int sock_fd;
    unsigned long nbytes, len;
    struct sockaddr_can addr;
    struct ifreq ifr;
    /*为了能够接收CAN报文，我们需要定义一个CAN数据格式的结构体变量*/
    struct can_frame frame;
    struct can_frame *ptr_frame;

    /*建立套接字，设置为原始套接字，原始CAN协议 */
    sock_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    /*以下是对CAN接口进行初始化，如设置CAN接口名，即当我们用ifconfig命令时显示的名字 */
    strcpy(ifr.ifr_name, can_dev);
    ioctl(sock_fd, SIOCGIFINDEX, &ifr);

#ifdef __DEBUG
    printf("can0 can_ifindex = %x\n", ifr.ifr_ifindex);
#endif

    /*设置CAN协议 */
    addr.can_family = AF_CAN;
    addr.can_ifindex = 0;

    /*将刚生成的套接字与网络地址进行绑定*/
    bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));

    while (run)
    {
        /*开始接收数据*/
        nbytes = recvfrom(sock_fd, (void *)&frame, sizeof(struct can_frame), 0, (struct sockaddr *)&addr, (socklen_t *)&len);
        if (nbytes > 0)
            exit_fun();

        /*get interface name of the received CAN frame*/
        ifr.ifr_ifindex = addr.can_ifindex;
        ioctl(sock_fd, SIOCGIFNAME, &ifr);

#ifdef __DEBUG
        printf("Received a CAN frame from interface %s\n", ifr.ifr_name);
#endif
        /*将接收到的CAN数据打印出来，其中ID为标识符，DLC为CAN的字节数，DATA为1帧报文的字节数*/
        printf("CAN frame:\n ID = %x\n DLC = %x\n" \
               "DATA = %s\n", frame.can_id, frame.can_dlc, frame.data);
        ptr_frame = &frame;

        // sleep;
        usleep(100000);
    }

    return 0;
}

int can_send(char *can_dev)
{
    int sock_fd;
    unsigned long nbytes;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

    /*建立套接字，设置为原始套接字，原始CAN协议 */
    sock_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    /*以下是对CAN接口进行初始化，如设置CAN接口名，即当我们用ifconfig命令时显示的名字 */
    strcpy((char *)(ifr.ifr_name), can_dev);
    ioctl(sock_fd, SIOCGIFINDEX, &ifr);

#ifdef __DEBUG
    printf("can0 can_ifindex = %x\n", ifr.ifr_ifindex);
#endif

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    /*将刚生成的套接字与CAN套接字地址进行绑定*/
    bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));

    /*设置CAN帧的ID号，可区分为标准帧和扩展帧的ID号*/
    frame.can_id = 0x1122;
    strcpy((char *)frame.data, "hello");
    frame.can_dlc = strlen(frame.data);

#ifdef __DEBUG
    printf("Send a CAN frame from interface %s\n", ifr.ifr_name);
#endif

    /*开始发送数据*/
    nbytes = sendto(sock_fd, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
    usleep(500000);
    nbytes = sendto(sock_fd, &frame, sizeof(struct can_frame), 0, (struct sockaddr*)&addr, sizeof(addr));
    usleep(500000);
    return 0;
}

int main(void)
{
    pid_t read_pid, send_pid;

    system("canconfig can0 bitrate 1000000 ctrlmode triple-sampling on restart-ms 1000 ");
    system("canconfig can0 start");
    system("canconfig can1 bitrate 1000000 ctrlmode triple-sampling on restart-ms 1000 ");
    system("canconfig can1 start");

    usleep(500000);

    send_pid = fork();
    if (send_pid < 0)
    {
        perror("error fork sendfork");
    }
    else if (send_pid == 0)
    {
        can_recv("can1");
        system("canconfig can1 stop");
        exit(1);
    }

    read_pid = fork();
    if (read_pid < 0)
    {
        perror("error fork readfork");
    }
    else if (read_pid == 0)
    {
        can_send("can0");
        system("canconfig can0 stop");
        exit(1);
    }

    return 0;
}

