//
// Created by cdy on 23-9-28.
//

#ifndef TALON_RPC_WAKEUP_FD_EVENT_H
#define TALON_RPC_WAKEUP_FD_EVENT_H

#include "fd_event.h"

namespace talon {
    class WakeUpFdEvent : public Fd_Event {
    public:
        WakeUpFdEvent(int fd);

        ~WakeUpFdEvent();

        void wakeup();

    private:

    };

}

#endif //TALON_RPC_WAKEUP_FD_EVENT_H
