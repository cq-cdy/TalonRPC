//
// Created by cdy on 23-9-29.
//

#ifndef TALON_RPC_TIMER_H
#define TALON_RPC_TIMER_H
#include "fd_event.h"
#include "map"
#include "mutex.h"
#include "timer_event.h"
namespace talon {

    class Timer : public Fd_Event {
    public:

        Timer();

        ~Timer();

        void addTimerEvent(const TimerEvent::s_ptr& event);

        void deleteTimerEvent(const TimerEvent::s_ptr& event);

        void onTimer(); // 当发送了 IO 事件后，eventloop 会执行这个回调函数

    private:
        void resetArriveTime();

    private:
        std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
        Mutex m_mutex;

    };


}  // namespace talon

#endif  // TALON_RPC_TIMER_H
