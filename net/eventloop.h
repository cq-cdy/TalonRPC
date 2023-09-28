//
// Created by cdy on 23-9-27.
//

#ifndef TALON_RPC_EVENTLOOP_H
#define TALON_RPC_EVENTLOOP_H

#include "pthread.h"
#include "set"
#include "functional"
#include "queue"
#include "mutex"
#include "thread"
#include "fd_event.h"
#include "wakeup_fd_event.h"
namespace talon{
    class Eventloop {
    public:
        Eventloop();
        ~Eventloop();
        void loop();
        void wakeup();
        void stop();
        void addEpollEvent(Fd_Event* event);
        void deleteEpollEvent(Fd_Event* event);
        bool isInLoopThread() const;
        void addTask(std::function<void()>,bool a = false );
    private:
        void dealWakeup();
        void initWakeUpFdEevent();
    private:
        WakeUpFdEvent* m_wakeup_fd_event {nullptr};
        pid_t m_thread_id{0};
        int m_epoll_fd{0};
        int m_wakeup_fd{0};
        bool m_stop_flag;
        std::set<int> m_listen_fds;
        std::queue<std::function<void()>> m_pending_tasks;
        static std::mutex m_mtx;
    };
}



#endif //TALON_RPC_EVENTLOOP_H
