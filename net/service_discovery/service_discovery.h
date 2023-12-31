//
// Created by cdy on 23-10-20.
//
#include <arpa/inet.h>
#include "string"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void sys_error(const char *str) {
    perror(str);
    exit(1);
}

std::string serviceDiscovery( const std::string& service_funcName,const std::string& service_center_ip,int query_port) {
    int cfd, ret;

    cfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(query_port);
    ret = inet_pton(AF_INET, service_center_ip.c_str(), &serv_addr.sin_addr.s_addr);
    if (cfd == -1) {
        sys_error("socket error");
    }
    // 连接服务发现 模块
    ret = connect(cfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (ret != 0) {
        sys_error("connect error");
    }

    ret = write(cfd, service_funcName.c_str(), service_funcName.length());
    if (ret < 0) {
        sys_error("write error");
    }
    char addr_buff[1024];
    ret = read(cfd, addr_buff, sizeof(addr_buff));
    if ( ret< 0) {
        sys_error("read error");
    }
    addr_buff[ret] = '\0';
    close(cfd);
    return addr_buff;


}