//
// Created by cdy on 23-9-30.
//

#include "iothread.h"
#include <cassert>
#include "log.h"
#include "util.h"
talon::IOThread::IOThread() {
    assert((sem_init(&m_init_semaphore,0,0) == 0));
    assert((sem_init(&m_start_semaphore,0,0) == 0));

    pthread_create(&m_thread, nullptr,&IOThread::Main,this);

    // wait, 直到新线程执行完 Main 函数的前置
    sem_wait(&m_init_semaphore);
    DEBUGLOG("IOThread [%d] create success", m_thread_id);
}

talon::IOThread::~IOThread() {
    if(m_event_loop != nullptr){
        m_event_loop->stop();
    }
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    pthread_join(m_thread, nullptr);

    if (m_event_loop) {
        delete m_event_loop;
        m_event_loop = nullptr;
    }
}

talon::Eventloop *talon::IOThread::getEventLoop() {
    return m_event_loop;
}

void talon::IOThread::start() {
    DEBUGLOG("Now invoke IOThread %d", m_thread_id);
    sem_post(&m_start_semaphore);
}

void talon::IOThread::join() const {
    pthread_join(m_thread, nullptr);
}

//虽然 main方法是静态的，但是传进来的arg对象是不一样的
/**
 *
 *
 * @details 这样的线程函数设计让我学到了很多
 * 首先，是将主线程中的IOThread 通过arg
 * 传递给线程函数，即新的线程。主线程的iothread
 * 对象就可以在主线程通过线程中的指针，来给
 * 主线程中的IOThread的成员指针赋值
 */
void *talon::IOThread::Main(void *arg) {

    auto* thread = static_cast<IOThread*>(arg);//指向主线程里面的IOThread对象实例
    thread->m_event_loop = new Eventloop(); // 使得主线程可以拿到子线程的Eventloop实例
    thread->m_thread_id = get_thread_id();
    sem_post(&thread->m_init_semaphore);
    DEBUGLOG("IOThread %d created, wait start semaphore", thread->m_thread_id);
    sem_wait(&thread->m_start_semaphore);
    DEBUGLOG("IOThread %d start loop ", thread->m_thread_id);
    thread->m_event_loop->loop();
    DEBUGLOG("IOThread %d end loop ", thread->m_thread_id);
    return nullptr;
}
