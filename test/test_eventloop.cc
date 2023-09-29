//
// Created by cdy on 23-9-29.
//

#include "config.h"
#include "eventloop.h"
#include "fd_event.h"
#include "log.h"
#include "wakeup_fd_event.h"
// 导入 socket 相关的头文件
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
using namespace talon;
int main() {
    Config::SetGlobalConfig("../conf/talon.xml");
    Logger::InitGlobalLogger();

    auto* eventloop = new Eventloop();
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        ERRORLOG("socket error");
        return -1;
    }
    sockaddr_in addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listen_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ERRORLOG("bind error");
        return -1;
    }
    if (listen(listen_fd, 5) < 0) {
        ERRORLOG("listen error");
        return -1;
    }

    Fd_Event fd_event(listen_fd);
    fd_event.listen(Fd_Event::TriggerEvent::IN_EVENT, [listen_fd, eventloop]() {
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        DEBUGLOG("before accept")
        int client_fd =
            accept(listen_fd, (sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            ERRORLOG("accept error");
            return;
        }
        DEBUGLOG("after accept = %d", client_fd);
        INFOLOG("accept a new connection from %s:%d",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    });
    DEBUGLOG("accept_fd = %d", listen_fd);
    eventloop->addEpollEvent(&fd_event);
    eventloop->loop();
    return 0;
}