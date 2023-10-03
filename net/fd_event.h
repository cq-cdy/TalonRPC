//
// Created by cdy on 23-9-28.
//

#ifndef TALON_RPC_FD_EVENT_H
#define TALON_RPC_FD_EVENT_H

#include "sys/epoll.h"
#include "functional"

namespace talon {


    class Fd_Event {

    public:
        enum TriggerEvent{
            IN_EVENT = EPOLLIN,
            OUT_EVENT = EPOLLOUT,
        };
        Fd_Event();
        Fd_Event(int fd);
        ~Fd_Event();
        void setNonBlock();
        void cancle(TriggerEvent);
        std::function<void()> handler(TriggerEvent event_type);
        void listen(TriggerEvent event_type,std::function<void()>);
        int getFd() const{
            return m_fd;
        }
        epoll_event getEpollEvent(){
            return m_listen_event;
        }

    protected:
        int m_fd {-1};
    private:

        epoll_event m_listen_event{};
        std::function<void()> m_read_callback;
        std::function<void()> m_write_callback;

    };
}


#endif //TALON_RPC_FD_EVENT_H
