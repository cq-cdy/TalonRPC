//
// Created by cdy on 23-9-28.
//

#include "fd_event.h"
#include "fcntl.h"
#include "unistd.h"
#include <utility>
#include "log.h"
#include "cstring"

namespace  talon
{


    Fd_Event::Fd_Event(int fd) : m_fd(fd) {
        memset(&m_listen_event, 0, sizeof(m_listen_event));
    }


    Fd_Event::Fd_Event() {
        memset(&m_listen_event, 0, sizeof(m_listen_event));
    }



    Fd_Event::~Fd_Event() {

    }


    std::function<void()> Fd_Event::handler(TriggerEvent event) {
        if (event == TriggerEvent::IN_EVENT) {
            return m_read_callback;
        } else if (event == TriggerEvent::OUT_EVENT) {
            return m_write_callback;
        }else if (event == TriggerEvent::ERROR_EVENT){
            return m_err_callback;
        }
        return nullptr;
    }


    void Fd_Event::listen(TriggerEvent event_type, const std::function<void()>& callback,
                          const std::function<void()>& error_callback) {
        if (event_type == TriggerEvent::IN_EVENT) {
            m_listen_event.events |= EPOLLIN;
            m_read_callback = callback;
        } else {
            m_listen_event.events |= EPOLLOUT;
            m_write_callback = callback;
        }

        if (m_err_callback == nullptr) {
            m_err_callback = error_callback;
        } else {
            m_err_callback = nullptr;
        }

        m_listen_event.data.ptr = this;
    }


    void Fd_Event::cancle(TriggerEvent event_type) {
        if (event_type == TriggerEvent::IN_EVENT) {
            m_listen_event.events &= (~EPOLLIN);
        } else {
            m_listen_event.events &= (~EPOLLOUT);
        }
    }


    void Fd_Event::setNonBlock() {

        int flag = fcntl(m_fd, F_GETFL, 0);
        if (flag & O_NONBLOCK) {
            return;
        }

        fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
    }
}



