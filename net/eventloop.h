//
// Created by cdy on 23-9-27.
//

#ifndef TALON_RPC_EVENTLOOP_H
#define TALON_RPC_EVENTLOOP_H

#include "pthread.h"
#include "set"
namespace talon{
    class Eventloop {
    public:
        Eventloop();
        ~Eventloop();
        void loop();
        void wakeup();
        void stop();
    private:
        pid_t m_pid;
        int m_epoll_fd{};
        std::set<int> m_linsten_fds;
    };
}



#endif //TALON_RPC_EVENTLOOP_H
