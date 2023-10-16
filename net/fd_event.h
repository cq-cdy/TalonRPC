//
// Created by cdy on 23-9-28.
//

#ifndef TALON_RPC_FD_EVENT_H
#define TALON_RPC_FD_EVENT_H

#include "functional"
#include "sys/epoll.h"

namespace talon {

class Fd_Event {
   public:
    enum TriggerEvent {
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT,
        ERROR_EVENT = EPOLLERR,
    };
    Fd_Event();
    Fd_Event(int fd);
    ~Fd_Event();
    void setNonBlock();
    void cancle(TriggerEvent);
    std::function<void()> handler(TriggerEvent event_type);
    void listen(TriggerEvent event_type, const std::function<void()>&,
                const std::function<void()>& error_callback = nullptr);
    int getFd() const { return m_fd; }
    epoll_event getEpollEvent() { return m_listen_event; }

   protected:
    int m_fd{-1};

   private:
    epoll_event m_listen_event{};
    std::function<void()> m_read_callback{nullptr};
    std::function<void()> m_write_callback{nullptr};
    std::function<void()> m_err_callback{nullptr};
};
}  // namespace talon

#endif  // TALON_RPC_FD_EVENT_H
