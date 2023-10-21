//
// Created by cdy on 23-10-20.
//
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>

const int MAX_EVENTS = 10;
const int SERVER_PORT = 8080;
const int CONTROL_PORT = 9090;

std::map<std::string, std::string> service_map;
//    {"Order.makeOrder", "127.0.0.1:12345"},
//    {"Order.queryOrder", "127.0.0.1:12346"}};

int main() {
    int serverSocket, controlSocket, clientSocket;
    struct sockaddr_in serverAddr {
    }, controlAddr{}, clientAddr{};
    socklen_t serverAddrLen = sizeof(clientAddr);
    socklen_t controlAddrLen = sizeof(clientAddr);
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[1024];
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating server socket" << std::endl;
        return 1;
    }
    controlSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (controlSocket == -1) {
        std::cerr << "Error creating control socket" << std::endl;
        close(serverSocket);
        return 1;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    controlAddr.sin_family = AF_INET;
    controlAddr.sin_port = htons(CONTROL_PORT);
    controlAddr.sin_addr.s_addr = INADDR_ANY;

    int val = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) !=
        0) {
        printf("setsockopt error\n");
    }
    int val2 = 1;
    if (setsockopt(controlSocket, SOL_SOCKET, SO_REUSEADDR, &val2,
                   sizeof(val2)) != 0) {
        printf("setsockopt error\n");
    }
    if (bind(serverSocket, reinterpret_cast<const sockaddr *>(&serverAddr),
             serverAddrLen) == -1) {
        std::cerr << "Error binding to the server socket" << std::endl;
        close(serverSocket);
        close(controlSocket);
        return 1;
    }

    if (bind(controlSocket, reinterpret_cast<const sockaddr *>(&controlAddr),
             controlAddrLen) == -1) {
        std::cerr << "Error binding to the control socket" << std::endl;
        close(serverSocket);
        close(controlSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error listening on the server socket" << std::endl;
        close(serverSocket);
        close(controlSocket);
        return 1;
    }

    if (listen(controlSocket, 5) == -1) {
        std::cerr << "Error listening on the control socket" << std::endl;
        close(serverSocket);
        close(controlSocket);
        return 1;
    }

    // epoll
    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Error creating epoll" << std::endl;
        close(serverSocket);
        close(controlSocket);
        return 1;
    }

    struct epoll_event event {};
    event.data.fd = serverSocket;
    event.events = EPOLLIN;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) < 0) {
        std::cerr << "Error adding server socket to epoll" << std::endl;
        close(serverSocket);
        close(controlSocket);
        close(epollFd);
        return 1;
    }

    event.data.fd = controlSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, controlSocket, &event) == -1) {
        std::cerr << "Error adding control socket to epoll" << std::endl;
        close(serverSocket);
        close(controlSocket);
        close(epollFd);
        return 1;
    }

    while (true) {
        struct epoll_event events[MAX_EVENTS];
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == serverSocket) {
                clientSocket = ::accept(
                    serverSocket, reinterpret_cast<sockaddr *>(&controlAddr),
                    &serverAddrLen);
                printf("c = %d \n", clientSocket);
                if (clientSocket < 0) {
                    std::cerr << "Error accepting client connection"
                              << std::endl;
                    continue;
                }
                event.data.fd = clientSocket;
                event.events = EPOLLIN;
                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) <
                    0) {
                    std::cerr << "Error adding client socket to epoll"
                              << std::endl;
                    close(clientSocket);
                }
            } else if (events[i].data.fd == controlSocket) {
                clientSocket =
                    accept(controlSocket, (struct sockaddr *)&clientAddr,
                           &clientAddrLen);
                if (clientSocket == -1) {
                    std::cerr << "Error accepting control connection"
                              << std::endl;
                    continue;
                }
                size_t bytesRead =
                    recv(clientSocket, buffer, sizeof(buffer), 0);
                if (bytesRead <= 0) {
                    // Connection closed or error
                    close(clientSocket);
                } else {
                    buffer[bytesRead] = '\0';
                    // todo Process the command (add, delete, lookup, etc.) and
                    // update serviceMap
                    std::string command = buffer;
                    std::string operator_c =
                        command.substr(0, command.find_first_of(' '));
                    std::string back_message;
                    std::string content = command.substr(
                        command.find_first_of(' ') + 1, command.length());
                        printf("operator_c = %s\n", operator_c.c_str());
                        printf("content = %s\n", content.c_str());
                    if (operator_c == "add") {
                        std::string service_name =
                            content.substr(0, content.find_first_of(' '));
                        std::string service_addr = content.substr(
                            content.find_first_of(' ') + 1, content.length());
                        printf("service_name = %s\n", service_name.c_str());
                        printf("service_addr = %s\n", service_addr.c_str());
                        service_map[service_name] = service_addr;
                        back_message = "add service success";
                        if (write(clientSocket, back_message.c_str(),
                                  back_message.length()) < 0) {
                            perror("write back error");
                        }
                    } else if (operator_c == "delete") {
                        std::string service_name = content;
                        if (service_map.find(service_name) ==
                            service_map.end()) {
                            back_message = "service not exist";
                            if (write(clientSocket, back_message.c_str(),
                                      back_message.length()) < 0) {
                                perror("write error");
                            }
                        } else {
                            service_map.erase(service_name);
                            back_message = "delete service success";
                            if (write(clientSocket, back_message.c_str(),
                                      back_message.length()) < 0) {
                                perror("write error");
                            }
                        }
                    } else if (operator_c == "lookup") {
                        if (service_map.empty()) {
                            back_message = "service map is empty";
                            if (write(clientSocket, back_message.c_str(),
                                      back_message.length()) < 0) {
                                perror("write error");
                            }
                        } else {
                            for (auto i : service_map) {
                                back_message += i.first + " " + i.second + "\n";
                            }
                            int ret = write(clientSocket, back_message.c_str(),
                                            back_message.length());
                            if (ret < 0) {
                                perror("write error");
                            }
                        }

                    } else if (operator_c == "modify") {
                        std::string service_name =
                            content.substr(0, content.find_first_of(' '));
                        std::string service_addr = content.substr(
                            content.find_first_of(' ') + 1, content.length());
                        if (service_map.find(service_name) ==
                            service_map.end()) {

                            back_message = "service not exist";
                            if (write(clientSocket, back_message.c_str(),
                                      back_message.length()) < 0) {
                                perror("write error");
                            }
                        } else {
                            service_map[service_name] = service_addr;
                            back_message = "modify service success";
                            if (write(clientSocket, back_message.c_str(),
                                      back_message.length()) < 0) {
                                perror("write error");
                            }
                        }
                    } else {
                        back_message = "unkown command";
                        if (write(clientSocket, back_message.c_str(),
                                  back_message.length()) < 0) {
                            perror("write error");
                        }
                    }
                }
            } else {
                // todo Handle client connections
                size_t byteRead =
                    recv(events[i].data.fd, buffer, sizeof(buffer) - 1,
                         0);  // 为'\0'留下一个位置

                if (byteRead <= 0) {
                    close(events[i].data.fd);
                } else {
                    buffer[byteRead] = '\0';  // 保证字符串结尾有一个null字符s
                    std::string server_name = buffer;
                    std::string service_addr;
                    if (service_map.find(server_name) == service_map.end()) {
                        service_addr = "unknown host";
                    } else {
                        service_addr = service_map[server_name];
                    }
                    printf("get service name = %s,return addr = %s\n",
                           server_name.c_str(), service_addr.c_str());
                    ssize_t ret = write(events[i].data.fd, service_addr.c_str(),
                                        service_addr.length());
                    if (ret < 0) {
                        perror("write error");  // 打印具体的错误信息
                    } else if (static_cast<size_t>(ret) !=
                               service_addr.length()) {
                        std::cerr << "Not all bytes written. Expected: "
                                  << service_addr.length()
                                  << ", Written: " << ret << std::endl;
                    }
                }
            }
        }
    }
}