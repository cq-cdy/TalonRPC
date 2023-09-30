//
// Created by cdy on 23-9-30.
//

#ifndef TALON_RPC_IOTHREAD_H
#define TALON_RPC_IOTHREAD_H

#include "thread"
#include "semaphore.h"
#include "eventloop.h"

namespace talon {
    class IOThreadGroup;
    class IOThread {

    public:
        friend IOThreadGroup;
        IOThread();

        ~IOThread();

        Eventloop *getEventLoop();


    public:


    private:
        static void *Main(void *arg);
        void start();

        void join() const;

    private:
        pid_t m_thread_id{-1};
        pthread_t m_thread{0};

        Eventloop *m_event_loop{nullptr};
        sem_t m_init_semaphore{};
        sem_t m_start_semaphore{};
    };

}


#endif //TALON_RPC_IOTHREAD_H
