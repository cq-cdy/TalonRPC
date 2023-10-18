//
// Created by cdy on 23-9-29.
//

#include "timer.h"

#include "cstring"
#include "log.h"
#include "sys/timerfd.h"
#include "util.h"

namespace talon {
    Timer::Timer() : Fd_Event() {


        m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        DEBUGLOG("timer fd=%d", m_fd);

        // 把 fd 可读事件放到了 eventloop 上监听
        listen(Fd_Event::IN_EVENT, [this] { onTimer(); });
    }

    Timer::~Timer() {
    }


    void Timer::onTimer() {
        // 处理缓冲区数据，防止下一次继续触发可读事件
        // DEBUGLOG("ontimer");
        char buf[8];
        while(1) {
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
                    tasks.emplace_back((*it).second->getArriveTime(), (*it).second->getCallBack());
                }
            } else {
                break;
            }
        }

        m_pending_events.erase(m_pending_events.begin(), it);
        lock.unlock();


        // 需要把重复的Event 再次添加进去
        for (auto & tmp : tmps) {
            if (tmp->isRepeated()) {
                // 调整 arriveTime
                tmp->resetArriveTime();
                addTimerEvent(tmp);
            }
        }

        resetArriveTime();

        for (const auto& i: tasks) {
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
            ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno));
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

        DEBUGLOG("success delete TimerEvent at arrive time %lld", event->getArriveTime());

    }


}  // namespace talon
