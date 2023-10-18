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
#include "mutex.h"

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

        void addTask(const std::function<void()>& cb, bool is_wake_up = false);

        void addTimerEvent(const TimerEvent::s_ptr& event);

        bool isLooping();

    public:
        static Eventloop* GetCurrentEventLoop();


    private:
        void dealWakeup();

        void initWakeUpFdEevent();

        void initTimer();

    private:
        pid_t m_thread_id {0};

        int m_epoll_fd {0};

        int m_wakeup_fd {0};

        WakeUpFdEvent* m_wakeup_fd_event {nullptr};

        bool m_stop_flag {false};

        std::set<int> m_listen_fds;

        std::queue<std::function<void()>> m_pending_tasks;

        Mutex m_mutex;

        Timer* m_timer {nullptr};

        bool m_is_looping {false};

    };

}



#endif //TALON_RPC_EVENTLOOP_H
