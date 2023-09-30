//
// Created by cdy on 23-9-29.
//

#include "config.h"
#include "eventloop.h"
#include "fd_event.h"
#include "log.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "memory"
#include "timer_event.h"
#include <cstring>
#include "iothread.h"
#include "iothreadgroup.h"

using namespace talon;

void test_io_thread() {
    /**
     *
     * 并不是一定要y
     */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr{};
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);

    int rt = bind(listenfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
    if (rt != 0) {
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(listenfd, 100);
    if (rt != 0) {
        ERRORLOG("listen error");
        exit(1);
    }


    // test net_io event
    talon::Fd_Event net_event(listenfd);
    net_event.listen(talon::Fd_Event::IN_EVENT, [listenfd]() {
        sockaddr_in peer_addr{};
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr *>(&peer_addr), &addr_len);

        INFOLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr),
                 ntohs(peer_addr.sin_port));

    });


    // test timer event
    int i = 0;
    talon::TimerEvent::s_ptr timer_event = std::make_shared<talon::TimerEvent>(
            1000, true, [&i]() {
                INFOLOG("trigger timer event, count=%d", i++);
            }
    );


    // talon::IOThread io_thread;


    // io_thread.getEventLoop()->addEpollEvent(&event);
    // io_thread.getEventLoop()->addTimerEvent(timer_event);
    // io_thread.start();

    // io_thread.join();

    talon::IOThreadGroup io_thread_group(3);
    std::vector<IOThread *> &ios = io_thread_group.getIOThread_group();

    for (const auto &io: ios) {
        io->getEventLoop()->addEpollEvent(&net_event); // 监听网络端口 连接事件
        io->getEventLoop()->addTimerEvent(timer_event); // Timer Event 事件
    }
    io_thread_group.start();
    io_thread_group.join();
//    talon::IOThread* io_thread = io_thread_group.getIOThread();
//    // io_thread->getEventLoop()->addEpollEvent(&event);
//    io_thread->getEventLoop()->addTimerEvent(timer_event);
//
//    talon::IOThread* io_thread2 = io_thread_group.getIOThread();
//    io_thread2->getEventLoop()->addTimerEvent(timer_event);
//
//    io_thread_group.start();
//
//    io_thread_group.join();


}

int main() {
    Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();

    test_io_thread();
//    Config::SetGlobalConfig("../conf/talon.xml");
//    Logger::InitGlobalLogger();
//
//    auto *eventloop = new Eventloop();
//    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
//    if (listen_fd < 0) {
//        ERRORLOG("socket error");
//        return -1;
//    }
//    sockaddr_in addr{};
//    memset(&addr, 0, sizeof(addr));
//    addr.sin_family = AF_INET;
//    addr.sin_port = htons(12345);
//    addr.sin_addr.s_addr = htonl(INADDR_ANY);
//    if (bind(listen_fd, (sockaddr *) &addr, sizeof(addr)) < 0) {
//        ERRORLOG("bind error");
//        return -1;
//    }
//    if (listen(listen_fd, 5) < 0) {
//        ERRORLOG("listen error");
//        return -1;
//    }
//
//    Fd_Event fd_event(listen_fd);
//    fd_event.listen(Fd_Event::TriggerEvent::IN_EVENT, [listen_fd, eventloop]() {
//        sockaddr_in client_addr{};
//        socklen_t client_addr_len = sizeof(client_addr);
//        DEBUGLOG("before accept")
//        int client_fd =
//                accept(listen_fd, (sockaddr *) &client_addr, &client_addr_len);
//        if (client_fd < 0) {
//            ERRORLOG("accept error");
//            return;
//        }
//        DEBUGLOG("after accept = %d", client_fd);
//        INFOLOG("accept a new connection from %s:%d",
//                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
//    });
//    DEBUGLOG("accept_fd = %d", listen_fd);
//    eventloop->addEpollEvent(&fd_event);
//    int i = 0;
//    auto timer_event = std::make_shared<talon::TimerEvent>(
//            1000, true, [&i]() {
//                INFOLOG("tirgger timer event,count = %d", i++);
//            }
//    );
//    eventloop->addTimerEvent(timer_event);
//    auto timer_event2 = std::make_shared<talon::TimerEvent>(
//            2000, true, [&i]() {
//                INFOLOG("i am timer task2");
//            }
//    );
//    eventloop->addTimerEvent(timer_event2);
//    eventloop->loop();
//    return 0;
}