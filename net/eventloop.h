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
#include "timer.h"
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
        void addTask(const std::function<void()>&,bool a = false);
        void addTimerEvent(const TimerEvent::s_ptr&);
        static Eventloop* GetCurrentEventLoop();
        bool isLooping() const;
    private:
        void dealWakeup();
        void initWakeUpFdEevent();
        void initTimer();

    private:
        WakeUpFdEvent* m_wakeup_fd_event {nullptr};
        pid_t m_thread_id{0};
        int m_epoll_fd{0};
        int m_wakeup_fd{0};
        bool m_stop_flag{};
        std::set<int> m_listen_fds;
        std::queue<std::function<void()>> m_pending_tasks;
        std::mutex m_mtx;
        Timer* m_timer{nullptr};
        bool m_is_loop{false};

    };
}



#endif //TALON_RPC_EVENTLOOP_H
