//
// Created by cdy on 23-9-30.
//

#ifndef TALON_RPC_IOTHREAD_H
#define TALON_RPC_IOTHREAD_H

#include "thread"
#include "semaphore.h"
#include "eventloop.h"

namespace talon {

    class IOThread {
    public:
        IOThread();

        ~IOThread();

        Eventloop* getEventLoop();

        void start();

        void join() const;

    public:
        static void* Main(void* arg);


    private:
        pid_t m_thread_id {-1};    // 线程号
        pthread_t m_thread {0};   // 线程句柄

        Eventloop* m_event_loop {nullptr}; // 当前 io 线程的 loop 对象

        sem_t m_init_semaphore;

        sem_t m_start_semaphore;

    };


}


#endif //TALON_RPC_IOTHREAD_H
