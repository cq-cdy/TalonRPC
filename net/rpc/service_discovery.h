//
// Created by cdy on 23-10-20.
//
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


// 服务发现的地址和端口
#define DISCOVERY_PORT 8080
#define DISCOVERY_IP "127.0.0.1"

void sys_error(const char *str) {
    perror(str);
    exit(1);
}

std::string serviceDiscovery(const std::string &service_funcName) {

    int cfd, ret;
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    // 初始化待连接的服务器信息
    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(DISCOVERY_PORT);
    ret = inet_pton(AF_INET, DISCOVERY_IP, &serv_addr.sin_addr.s_addr);
    if (cfd == -1) {
        sys_error("socket error");
    }
    // 连接服务发现 模块
    ret = connect(cfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (ret != 0) {
        sys_error("connect error");
    }
    char buf[1024];
    memcpy(buf,service_funcName.c_str(),service_funcName.length());
    if(write(cfd, buf, service_funcName.length()) < 0){
        sys_error("write error");
    }
    if(read(cfd, buf, sizeof(buf))<0){
        sys_error("read error");

    }
    write(STDOUT_FILENO, buf, sizeof(1024));
    return std::string(buf);


}