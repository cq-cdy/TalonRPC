//
// Created by cdy on 23-9-29.
//

#include "timer.h"

#include "cstring"
#include "log.h"
#include "sys/timerfd.h"
#include "util.h"

namespace talon {
/*
Timer事件不是和普通的Fd_Event不一样
不一定是需要一个定时触发需求 就 产生一个Timer事件
而是 这个timer事件只有一个fd来描述，并且该对象维护
着一个任务队列，这个任务队列里面的任务都是定时任务
当timer_fd触发时，会执行任务队列里面已经到达时间的任务
所以，只需要往这个fd维护的任务队列里面添加任务即可。

*/
Timer::Timer() : Fd_Event() {
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    DEBUGLOG("timer fd=%d", m_fd);

    // 把 fd 可读事件放到了 eventloop 上监听
    listen(Fd_Event::IN_EVENT, [this] { onTimer(); });
}

Timer::~Timer() {}

void Timer::onTimer() {
    /*
       为什么要处理缓冲区?
      这是因为，如果不读取并清空这个缓冲区，
      那么该timerfd会持续处于一个"可读"的状态。
      这会导致对于任何依赖这个文件描述符状态的机制
      （如事件循环或多路复用系统如epoll、select等）来说，
      它会持续报告这个文件描述符是可读的，从而持续触发相应的回调或事件处理。
      */
    char buf[8];
    while (1) {
        if ((read(m_fd, buf, 8) == -1) && errno == EAGAIN) {
            break;
        }
    }

    // 执行定时任务
    int64_t now = get_now_ms();

    std::vector<TimerEvent::s_ptr> tmps;
    std::vector<std::pair<int64_t, std::function<void()>>> tasks;

    ScopeMutex<Mutex> lock(m_mutex);
    auto it = m_pending_events.begin();

    for (it = m_pending_events.begin(); it != m_pending_events.end(); ++it) {
        if ((*it).first <= now) {
            if (!(*it).second->isCancled()) {
                tmps.push_back((*it).second);
                tasks.emplace_back((*it).second->getArriveTime(),
                                   (*it).second->getCallBack());
            }
        } else {
            break;
        }
    }
    // 经过上面的for循环 此时的it已经指导最后一个超时而未执行的元素了
    // 由于上面已经将超时的元素都放到了tmps中，所以这里直接删除
    m_pending_events.erase(m_pending_events.begin(), it);
    lock.unlock();

    // 需要把重复的Event 再次添加进去
    for (auto& tmp : tmps) {
        if (tmp->isRepeated()) {
            // 调整 arriveTime
            tmp->resetArriveTime();
            addTimerEvent(tmp);
        }
    }

    resetArriveTime();  // 调整到下次timer_fd的触发时间，为任务队列里面最早的任务的时间

    for (const auto& i : tasks) {
        if (i.second) {
            i.second();
        }
    }
}

void Timer::resetArriveTime() {
    ScopeMutex<Mutex> lock(m_mutex);
    auto tmp = m_pending_events;
    lock.unlock();

    if (tmp.empty()) {
        return;
    }

    int64_t now = get_now_ms();

    auto it = tmp.begin();
    int64_t inteval = 0;
    if (it->second->getArriveTime() > now) {
        inteval = it->second->getArriveTime() - now;
    } else {
        inteval = 100;
    }

    timespec ts{};
    memset(&ts, 0, sizeof(ts));
    ts.tv_sec = inteval / 1000;
    ts.tv_nsec = (inteval % 1000) * 1000000;

    itimerspec value{};
    memset(&value, 0, sizeof(value));
    value.it_value = ts;

    int rt = timerfd_settime(m_fd, 0, &value, nullptr);
    if (rt != 0) {
        ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno,
                 strerror(errno));
    }
    // DEBUGLOG("timer reset to %lld", now + inteval);
}

void Timer::addTimerEvent(const TimerEvent::s_ptr& event) {
    bool is_reset_timerfd = false;

    ScopeMutex<Mutex> lock(m_mutex);
    if (m_pending_events.empty()) {
        is_reset_timerfd = true;
    } else {
        auto it = m_pending_events.begin();
        if ((*it).second->getArriveTime() > event->getArriveTime()) {
            is_reset_timerfd = true;
        }
    }
    m_pending_events.emplace(event->getArriveTime(), event);
    lock.unlock();

    if (is_reset_timerfd) {
        resetArriveTime();
    }
}

void Timer::deleteTimerEvent(const TimerEvent::s_ptr& event) {
    
    event->setCancled(true);

    ScopeMutex<Mutex> lock(m_mutex);

    auto begin = m_pending_events.lower_bound(event->getArriveTime());
    auto end = m_pending_events.upper_bound(event->getArriveTime());

    auto it = begin;
    for (it = begin; it != end; ++it) {
        if (it->second == event) {
            break;
        }
    }

    if (it != end) {
        m_pending_events.erase(it);
    }
    lock.unlock();

    DEBUGLOG("success delete TimerEvent at arrive time %lld",
             event->getArriveTime());
}

}  // namespace talon
