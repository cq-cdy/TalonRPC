//
// Created by cdy on 23-9-30.
//

#include "iothread.h"
#include <cassert>
#include "log.h"
#include "util.h"
namespace talon{
    IOThread::IOThread() {

        int rt = sem_init(&m_init_semaphore, 0, 0);
        assert(rt == 0);

        rt = sem_init(&m_start_semaphore, 0, 0);
        assert(rt == 0);

        pthread_create(&m_thread, nullptr, &IOThread::Main, this);

        // wait, 直到新线程执行完 Main 函数的前置
        sem_wait(&m_init_semaphore);

        DEBUGLOG("IOThread [%d] create success", m_thread_id);
    }

    IOThread::~IOThread() {

        m_event_loop->stop();
        sem_destroy(&m_init_semaphore);
        sem_destroy(&m_start_semaphore);

        pthread_join(m_thread, nullptr);

        if (m_event_loop) {
            delete m_event_loop;
            m_event_loop = nullptr;
        }
    }


    void* IOThread::Main(void* arg) {
        auto* thread = static_cast<IOThread*> (arg);

        thread->m_event_loop = new Eventloop();
        thread->m_thread_id = get_thread_id();


        // 唤醒等待的线程
        sem_post(&thread->m_init_semaphore);

        // 让IO 线程等待，直到我们主动的启动

        DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);

        sem_wait(&thread->m_start_semaphore);
        DEBUGLOG("IOThread %d start loop ", thread->m_thread_id);
        thread->m_event_loop->loop();

        DEBUGLOG("IOThread %d end loop ", thread->m_thread_id);

        return nullptr;

    }


    Eventloop* IOThread::getEventLoop() {
        return m_event_loop;
    }

    void IOThread::start() {
        DEBUGLOG("Now invoke IOThread %d", m_thread_id);
        sem_post(&m_start_semaphore);
    }


    void IOThread::join() const {
        pthread_join(m_thread, nullptr);
    }
}
