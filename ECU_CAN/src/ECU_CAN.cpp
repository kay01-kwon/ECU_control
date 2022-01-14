#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <iostream>

struct sockaddr_can addr;

using std::cout;
using std::endl;

int InitCanInterface(const char *ifname)
{
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if(sock == -1){
        printf("Fail to create CAN socket for %s - %m\n",ifname);
        return -1;
    }
    printf("Success to create CAN socket for %s \n",ifname);

    struct  ifreq ifr;
    strcpy(ifr.ifr_name, ifname);
    int ret = ioctl(sock, SIOCGIFINDEX, &ifr);

    if(ret == -1){
        perror("Fail to get CAN interface index -");
        return -1;
    }
    printf("Success to get CAN interface index: %d\n",ifr.ifr_ifindex);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    if(ret == -1)
        perror("Fail to bind CAN socket: -");
        
    return sock;
}

int HexarrayToInt(unsigned char *buffer,int length)
{
    int hextoint = 0;
    int data = 0;
    for(int i = 0; i < length; i++){
        hextoint += (buffer[i] << 8*i);
        data = (int) buffer[i];
        //printf("Index: %d \t Data: %02x \n",i,buffer[i]);
    }

    return hextoint;
}

int main()
{
    struct can_frame frame;
    struct canfd_frame frame_fd;
    char ctrlmsg[CMSG_SPACE(sizeof(struct timeval) + 3*sizeof(struct timespec) + sizeof(__u32))];
    struct iovec iov;
    struct msghdr can_msg;
    struct canfd_frame frame_get;

    int sock = InitCanInterface("slcan0");
    int nbytes = 0;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    iov.iov_base = &frame_get;
    can_msg.msg_name = & addr;
    can_msg.msg_iov = &iov;
    can_msg.msg_iovlen = 1;
    can_msg.msg_control = &ctrlmsg;

    iov.iov_len = sizeof(frame_get);
    can_msg.msg_namelen = sizeof(addr);
    can_msg.msg_controllen = sizeof(ctrlmsg);
    can_msg.msg_flags = 0;

    sleep(1);

    if(sock < 0)
        return -1;

    cout<<"Engine run"<<endl;

    // 07D0
    frame.can_id = 0x200;
    frame.can_dlc = 2;
    frame.data[0] = 0x07;
    frame.data[1] = 0xD0;

    write(sock,&frame,sizeof(can_frame));

    cout<<"Engine Start"<<endl;

    frame.can_id = 0x201;
    frame.can_dlc = 1;
    frame.data[0] = 0x01;
    write(sock,&frame,sizeof(can_frame));
    
    sleep(1);

while (1)
{

    nbytes = recvmsg(sock,&can_msg,0);
    if(frame_get.can_id == 0x100){
        cout<<std::hex<<(unsigned)frame_get.data[0]<<"  ";
        cout<<std::hex<<(unsigned)frame_get.data[1]<<"  ";
        cout<<std::hex<<(unsigned)frame_get.data[2]<<"  ";
        cout<<std::hex<<(unsigned)frame_get.data[3]<<"  ";
        cout<<std::hex<<(unsigned)frame_get.data[4]<<"  ";
        cout<<std::hex<<(unsigned)frame_get.data[5]<<endl;
    }
}


}