//
// Created by cdy on 23-9-29.
//

#ifndef TALON_RPC_TIMER_H
#define TALON_RPC_TIMER_H
#include "fd_event.h"
#include "map"
#include "mutex"
#include "timer_event.h"
namespace talon {

class Timer : public Fd_Event {
   public:
    Timer();

    ~Timer();

    void addTimerEvent(const TimerEvent::s_ptr& event);

    void deleteTimerEvent(const TimerEvent::s_ptr& event);

    void onTimer();  // 当发送了 IO 事件后，eventloop 会执行这个回调函数

   private:
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
    std::mutex m_mutex;
    void resetArriveTime();
};

}  // namespace talon

#endif  // TALON_RPC_TIMER_H
