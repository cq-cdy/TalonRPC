//
// Created by cdy on 23-9-27.
//

#include "eventloop.h"

#include <sys/eventfd.h>

#include <cstring>

#include "iostream"
#include "log.h"
#include "sys/epoll.h"
#include "sys/socket.h"
#include "util.h"

static int g_epoll_max_timeout = 10000;

static int g_epoll_max_events = 100;

#define ADD_TO_EPOLL()                                                       \
    auto it = m_listen_fds.find(event->getFd());                             \
    int op = EPOLL_CTL_ADD;                                                  \
    if (it != m_listen_fds.end()) {                                          \
        op = EPOLL_CTL_MOD;                                                  \
    }                                                                        \
    epoll_event tmp = event->getEpollEvent();                                \
    if ((epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp)) < 0) {             \
        ERRORLOG("failed epoll_ctl when add fd,errno = %d ,err = %s", errno, \
                 strerror(errno));                                           \
    }

#define DELETE_TO_EPOLL()                                                   \
    auto it = m_listen_fds.find(event->getFd());                            \
    if (it == m_listen_fds.end()) {                                         \
        return;                                                             \
    }                                                                       \
    int op = EPOLL_CTL_DEL;                                                 \
    epoll_event tmp = event->getEpollEvent();                               \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), NULL);               \
    if (rt == -1) {                                                         \
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s", errno, \
                 strerror(errno));                                          \
    }                                                                       \
    m_listen_fds.erase(event->getFd());                                     \
    DEBUGLOG("delete event success, fd[%d]", event->getFd());

namespace talon {
    std::mutex Eventloop::m_mtx;

    static thread_local Eventloop *t_current_eventloop = nullptr;

    Eventloop::Eventloop() {
        if (t_current_eventloop != nullptr) {
            ERRORLOG(
                    "failed to create event loop,this thread had created event loop");
            exit(Error);
        }
        m_thread_id = get_thread_id();

        m_epoll_fd = epoll_create(10);

        if (m_epoll_fd < 0) {
            ERRORLOG(
                    "failed to create event loop，epoll create failed err info[%d]",
                    errno);
            exit(Error);
        }
        DEBUGLOG("success create m_epoll_fd = %d",m_epoll_fd)
//        if ((m_wakeup_fd = eventfd(0, EFD_NONBLOCK)) < 0) {
//            ERRORLOG(
//                    "failed to create event loop，m_wakeup_fd create failed err "
//                    "info[%d]",
//                    errno);
//        }
//        DEBUGLOG("success create m_wakeup_fd = %d",m_wakeup_fd)
//        epoll_event event{};
//        event.events = EPOLLIN;
//        event.data.fd = m_wakeup_fd;
//        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_wakeup_fd, &event) < 0) {
//            ERRORLOG("failed to create event loop， eventfd add err info[%d]",
//                     errno);
//        }
        t_current_eventloop = this;
        initWakeUpFdEevent();


    }

    Eventloop::~Eventloop() {
        close(m_epoll_fd);
        if (m_p_wakeup_fd_event) {
            delete m_p_wakeup_fd_event;
            m_p_wakeup_fd_event = nullptr;
        }
    }

/*
 * epoll中，向epoll红黑树中添加一个非阻塞event，然后树上还有其他阻塞的event，
    那么当第一次epoll_wait()的时候，这个非阻塞event就不会等待，如果不把把这个非阻塞event从红黑树删除，
    那么while循环中的epoll_wait()永远不会阻塞等待，当其他阻塞的event响应式，就会和非阻塞event
    一起返回，如果第一次epoll_wait()的时候，把非阻塞event删除，
    那么epoll_wait()就会阻塞在这等待阻塞的event响应，
 * */
    void Eventloop::loop() {
        while (!m_stop_flag) {
            std::unique_lock<std::mutex> lock(m_mtx);
            auto tmp_tasks = m_pending_tasks;
            {
                std::queue<std::function<void()>> empty_queue;
                m_pending_tasks.swap(empty_queue);  // 快速清零
            } // 快速x
            lock.unlock();

            while (!tmp_tasks.empty()) {
                tmp_tasks.front()();
                tmp_tasks.pop();
            }
            int timeout = g_epoll_max_timeout;
            epoll_event result_events[g_epoll_max_events];
            DEBUGLOG("before epoll wait");
            int rt =
                    epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
            DEBUGLOG("after epoll wait,rt = %d",rt);

            if (rt < 0) {
                ERRORLOG("epoll wait error");
            } else {
                for (int i = 0; i < rt; i++) {
                    epoll_event trigger_event = result_events[i];
                    auto *fd_event =
                            static_cast<Fd_Event *>(trigger_event.data.ptr);
                    if (fd_event == nullptr) {
                        continue;
                    }
                    if (trigger_event.events | EPOLLIN) {
                        DEBUGLOG("fd trigger EPOLLIN")
                        addTask(fd_event->handler(Fd_Event::IN_EVENT));
                    } else if (trigger_event.events | EPOLLOUT) {
                        DEBUGLOG("fd  trigger EPOLLOUT")
                        addTask(fd_event->handler(Fd_Event::OUT_EVENT));
                    } else {
                        ERRORLOG("unknown trigger_event.events type")
                    }

                    int fd = fd_event->getFd();


                }
            }
        }
    }

    void Eventloop::wakeup() {
        INFOLOG("WAKE UP");
        if (m_p_wakeup_fd_event != nullptr) {
            m_p_wakeup_fd_event->wakeup();
        } else {
            ERRORLOG("m_p_wakeup_fd_event is nullptr");
        }
    }

    void Eventloop::stop() {}

    void Eventloop::dealWakeup() {}

    void Eventloop::addEpollEvent(Fd_Event *event) {
        if (isInLoopThread()) {
            ADD_TO_EPOLL()
            DEBUGLOG("add event success,fd[%d]", event->getFd());
        } else {
            auto call_back = [this, event]() { ADD_TO_EPOLL()};
            addTask(call_back);
        }
    }

    void Eventloop::deleteEpollEvent(Fd_Event *event) {
        if (isInLoopThread()) {
            DELETE_TO_EPOLL();
        } else {
            auto cb = [this, event]() { DELETE_TO_EPOLL(); };
            addTask(cb);
        }
    }

    bool Eventloop::isInLoopThread() const {  // 确定是loop线程
        return get_thread_id() == m_thread_id;
    }

    void Eventloop::addTask(const std::function<void()> task, bool is_wake_up) {
        std::scoped_lock lock(m_mtx);
        m_pending_tasks.push(task);
        if (is_wake_up) {
            wakeup();
        }
    }

    void Eventloop::initWakeUpFdEevent() {
        m_wakeup_fd = eventfd(0, EFD_NONBLOCK);
        if (m_wakeup_fd < 0) {
            ERRORLOG(
                    "failed to create event loop, eventfd create error, error info[%d]",
                    errno);
            exit(0);
        }
        INFOLOG("wakeup fd = %d", m_wakeup_fd);

        m_p_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
        if (m_p_wakeup_fd_event == nullptr) {
            ERRORLOG("m_p_wakeup_fd_event is nullptr");
        }
        m_p_wakeup_fd_event->listen(Fd_Event::IN_EVENT, [this]() {
            char buf[8];
            while (read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {
            }
            DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
        });

        addEpollEvent(m_p_wakeup_fd_event);
    }

}  // namespace talon
