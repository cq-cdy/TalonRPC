//
// Created by cdy on 23-10-21.
//

#ifndef TALON_RPC_RUN_TIME_SERVICE_DISCOVERY_H
#define TALON_RPC_RUN_TIME_SERVICE_DISCOVERY_H

#include <arpa/inet.h>
#include "string"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "string"


class RC_Message{
public:
    void set_info(std::string info) {
        this->info = info;
    }
    void set_is_success(bool is_success) {
        this->is_success = is_success;
    }
    std::string get_info() {
        return info;
    }
    bool get_is_success() {
        return is_success;
    }

private:
    std::string info;
    bool is_success {false};

};


class Runtime_Command {
public:
    Runtime_Command(std::string command_ip, int command_port) : m_command_ip(command_ip), m_command_port(command_port) {

    }

public:
    RC_Message modify_service(std::string service_name, std::string ip, int port) {
        std::string command = "modify" +std::string(" ") + service_name +std::string(" ")+ip+std::string(":")+std::string(std::to_string(port));
        RC_Message message;
        to_command(command,message);
        return message;
    }
    RC_Message delete_service(std::string service_name) {
        std::string command = "delete" +std::string(" ") + service_name;
        RC_Message message;
        to_command(command,message);
        return message;
    }
    RC_Message add_service(std::string service_name, std::string ip, int port) {
        std::string command = "add"+std::string(" ") +service_name+std::string(" ")+ip+std::string(":")+std::string(std::to_string(port));
        RC_Message message;
        to_command(command,message);
        return message;
    }

    RC_Message look_up() {
        std::string command = "lookup";
        RC_Message message;
        to_command(command,message);
        return message;
    }

private:
    void to_command(std::string command,RC_Message& message) {
        int cfd, ret;
        cfd = socket(AF_INET, SOCK_STREAM, 0);
        // 初始化待连接的服务器信息
        struct sockaddr_in serv_addr{};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(m_command_port);
        ret = inet_pton(AF_INET, m_command_ip.c_str(), &serv_addr.sin_addr.s_addr);
        if (cfd == -1) {
            std::string msg = "socket error";
            message.set_info(msg);
            perror(msg.c_str());
            return ;
        }
        // 连接服务发现 模块
        ret = connect(cfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
        if (ret != 0) {
            std::string msg = "connect error";
            message.set_info(msg);
            perror(msg.c_str());
            return;
        }
      
        ret = write(cfd, command.c_str(), command.length());
        if (ret < 0) {
            std::string msg = "write error";
            message.set_info(msg);
            perror(msg.c_str());
            return;
        }
        char buff[1024];
        ret = read(cfd, buff, sizeof(buff));
        if (ret < 0) {
            std::string msg = "read error";
            message.set_info(msg);
            perror(msg.c_str());
            return;
        }
        message.set_is_success(true);
        buff[ret] = '\0';
        message.set_info(buff);
        close(cfd);
    }
private:
    std::string m_command_ip;
    int m_command_port;
};


#endif //TALON_RPC_RUN_TIME_SERVICE_DISCOVERY_H
