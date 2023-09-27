//
// Created by cdy on 23-9-27.
//

#include "eventloop.h"
#include "sys/socket.h"
#include "eventloop.h"
#include "log.h"
#include  "util.h"
#include "sys/epoll.h"
namespace talon{

    static thread_local Eventloop* t_current_eventloop = nullptr;
    Eventloop::Eventloop() {
        if(t_current_eventloop != nullptr){
            ERRORLOG("failed to create event loop,this thread had created event loop");
            exit(Error);
        }
        m_pid = get_thread_id();
        INFOLOG("succeed create event loop in thread:%d",m_pid);
        m_epoll_fd = epoll_create(10);
        if(m_epoll_fd < 0 ){
            ERRORLOG("epoll create failed");
            exit(Error);
        }
        t_current_eventloop = this;
    }

    Eventloop::~Eventloop() {

    }

    void Eventloop::loop() {

    }

    void Eventloop::wakeup() {

    }

    void Eventloop::stop() {

    }
}