//
// Created by cdy on 23-9-28.
//

#include "wakeup_fd_event.h"
#include "unistd.h"
#include "log.h"
talon::WakeUpFdEvent::WakeUpFdEvent(int fd) : Fd_Event(fd) {

}

talon::WakeUpFdEvent::~WakeUpFdEvent() {

}

void talon::WakeUpFdEvent::wakeup() {
    char buf[8] = {'a'};
    int rt = write(Fd_Event::m_fd, buf, 8);
    if (rt != 8) {
        ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
    }
    DEBUGLOG("success read 8 bytes");
}
