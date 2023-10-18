//
// Created by cdy on 23-9-29.
//

#include "timer_event.h"

#include <utility>
#include "log.h"
#include "util.h"

namespace talon {
    TimerEvent::TimerEvent(int interval, bool is_repeated, std::function<void()> cb)
            : m_interval(interval), m_is_repeated(is_repeated), m_task(std::move(cb)) {
        resetArriveTime();
    }


    void TimerEvent::resetArriveTime() {

        m_arrive_time = get_now_ms() + m_interval;
        // DEBUGLOG("success create timer event, will excute at [%lld]", m_arrive_time);
    }
} // talon