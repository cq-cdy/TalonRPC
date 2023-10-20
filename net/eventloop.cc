//
// Created by cdy on 23-9-27.
//

#include "eventloop.h"

#include <sys/eventfd.h>

#include <cstring>

#include "iostream"
#include "log.h"
#include "sys/epoll.h"
#include "util.h"
#include "mutex.h"

#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if (it != m_listen_fds.end()) { \
      op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    INFOLOG("epoll_event.events = %d", (int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    DEBUGLOG("add event success, fd[%d]", event->getFd()) \


#define DELETE_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    if (it == m_listen_fds.end()) { \
      return; \
    } \
    int op = EPOLL_CTL_DEL; \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), nullptr); \
    if (rt == -1) { \
      ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, strerror(errno)); \
    } \
    m_listen_fds.erase(event->getFd()); \
    DEBUGLOG("delete event success, fd[%d]", event->getFd()); \

namespace talon {

    static thread_local Eventloop* t_current_Eventloop = nullptr;
    static int g_epoll_max_timeout = 10000;
    static int g_epoll_max_events = 10;

    Eventloop::Eventloop() {
        if (t_current_Eventloop != nullptr) {
            ERRORLOG("failed to create event loop, this thread has created event loop");
            exit(0);
        }
        m_thread_id = get_thread_id();

        m_epoll_fd = epoll_create(10);

        if (m_epoll_fd == -1) {
            ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno);
            exit(0);
        }

        initWakeUpFdEevent();
        initTimer();

        INFOLOG("succ create event loop in thread %d", m_thread_id);
        t_current_Eventloop = this;
    }

    Eventloop::~Eventloop() {
        close(m_epoll_fd);
        if (m_wakeup_fd_event) {
            delete m_wakeup_fd_event;
            m_wakeup_fd_event = nullptr;
        }
        if (m_timer) {
            delete m_timer;
            m_timer = nullptr;
        }
    }


    void Eventloop::initTimer() {
        m_timer = new Timer();
        addEpollEvent(m_timer);
    }

    void Eventloop::addTimerEvent(const TimerEvent::s_ptr& event) {
        m_timer->addTimerEvent(event);
    }

    void Eventloop::initWakeUpFdEevent() {
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
        if (m_wakeup_fd < 0) {
            ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno);
            exit(0);
        }
        INFOLOG("wakeup fd = %d", m_wakeup_fd);

        m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);

        m_wakeup_fd_event->listen(Fd_Event::IN_EVENT, [this]() {
            char buf[8];
            while(read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {
            }
            DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
        });

        addEpollEvent(m_wakeup_fd_event);

    }


    void Eventloop::loop() {
        m_is_looping = true;
        while(!m_stop_flag) {
            ScopeMutex<Mutex> lock(m_mutex);
            std::queue<std::function<void()>> tmp_tasks;
            m_pending_tasks.swap(tmp_tasks);
            lock.unlock();

            while (!tmp_tasks.empty()) {
                std::function<void()> cb = tmp_tasks.front();
                tmp_tasks.pop();
                if (cb) {
                    cb();
                }
            }

            // 如果有定时任务需要执行，那么执行
            // 1. 怎么判断一个定时任务需要执行？ （now() > TimerEvent.arrtive_time）
            // 2. arrtive_time 如何让 Eventloop 监听

            int timeout = g_epoll_max_timeout;
            epoll_event result_events[g_epoll_max_events];
            // DEBUGLOG("now begin to epoll_wait");
            int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
            // DEBUGLOG("now end epoll_wait, rt = %d", rt);

            if (rt < 0) {
                ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno));
            } else {
                for (int i = 0; i < rt; ++i) {
                    epoll_event trigger_event = result_events[i];
                    auto* fd_event = static_cast<Fd_Event*>(trigger_event.data.ptr);
                    if (fd_event == nullptr) {
                        ERRORLOG("fd_event = NULL, continue");
                        continue;
                    }

                    // int event = (int)(trigger_event.events);
                    // DEBUGLOG("unkonow event = %d", event);

                    if (trigger_event.events & EPOLLIN) {

                        DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd())
                        addTask(fd_event->handler(Fd_Event::IN_EVENT));
                    }
                    if (trigger_event.events & EPOLLOUT) {
                        DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd())
                        addTask(fd_event->handler(Fd_Event::OUT_EVENT));
                    }

                    // EPOLLHUP EPOLLERR
                    if (trigger_event.events & EPOLLERR) {
                        DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFd())
                        // 删除出错的套接字
                        deleteEpollEvent(fd_event);
                        if (fd_event->handler(Fd_Event::ERROR_EVENT) != nullptr) {
                            DEBUGLOG("fd %d add error callback", fd_event->getFd())
                            addTask(fd_event->handler(Fd_Event::OUT_EVENT));
                        }
                    }
                }
            }

        }

    }

    void Eventloop::wakeup() {
        INFOLOG("WAKE UP");
        m_wakeup_fd_event->wakeup();
    }

    void Eventloop::stop() {
        m_stop_flag = true;
        wakeup();
    }

    void Eventloop::dealWakeup() {

    }

    void Eventloop::addEpollEvent(Fd_Event* event) {

        /**
         * 如果 b
         */
        if (isInLoopThread()) {
            ADD_TO_EPOLL();
        } else {
            auto cb = [this, event]() {
                ADD_TO_EPOLL();
            };
            addTask(cb, true);
        }

    }

    void Eventloop::deleteEpollEvent(Fd_Event* event) {
        if (isInLoopThread()) {
            DELETE_TO_EPOLL();
        } else {

            auto cb = [this, event]() {
                DELETE_TO_EPOLL();
            };
            addTask(cb, true);
        }

    }

    void Eventloop::addTask(const std::function<void()>& cb, bool is_wake_up /*=false*/) {
        ScopeMutex<Mutex> lock(m_mutex);
        m_pending_tasks.push(cb);
        lock.unlock();

        if (is_wake_up) {
            wakeup();
        }
    }

    bool Eventloop::isInLoopThread() const {
        return get_thread_id() == m_thread_id;
    }
    Eventloop *Eventloop::GetCurrentEventLoop() {
        if (t_current_Eventloop) {
            return t_current_Eventloop;
        }
        t_current_Eventloop = new Eventloop();
        return t_current_Eventloop;
    }



    bool Eventloop::isLooping() const {
        return m_is_looping;
    }




}  // namespace talon