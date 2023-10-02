//
// Created by cdy on 23-9-28.
//

#include "fd_event.h"
#include "fcntl.h"
#include "unistd.h"
#include <utility>
#include "log.h"

talon::Fd_Event::Fd_Event(int fd) : m_fd(fd) {

}

talon::Fd_Event::~Fd_Event() {
}

std::function<void()> talon::Fd_Event::handler(talon::Fd_Event::TriggerEvent event_type) {
    if (event_type == IN_EVENT) {
        return this->m_read_call_back;
    } else if (event_type == OUT_EVENT) {
       return  this->m_write_call_back;
    } else {
        return [](){ ERRORLOG("unknown event_type")};
    }
}


void talon::Fd_Event::listen(talon::Fd_Event::TriggerEvent event_type, std::function<void()> cb) {
    m_listen_event.data.ptr = this;
    if (event_type == TriggerEvent::IN_EVENT) {
        m_listen_event.events |= EPOLLIN;
        m_read_call_back = std::move(cb);
    } else if (event_type == TriggerEvent::OUT_EVENT) {
        m_listen_event.events |= EPOLLIN;
        m_write_call_back = std::move(cb);
    }else{
        ERRORLOG("unknown event_type")
    }
}

talon::Fd_Event::Fd_Event() {

}

void talon::Fd_Event::setNonBlock() {
    int flag = fcntl(m_fd,F_GETFL,0);
    if(flag & O_NONBLOCK){
        return;
    }
    fcntl(m_fd,F_SETFL,flag|O_NONBLOCK);
}

void talon::Fd_Event::cancle(talon::Fd_Event::TriggerEvent event_type) {
    if (event_type == TriggerEvent::IN_EVENT) {
        m_listen_event.events &= (~EPOLLIN);
    } else {
        m_listen_event.events &= (~EPOLLOUT);
    }
}



