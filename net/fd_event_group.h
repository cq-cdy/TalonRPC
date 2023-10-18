//
// Created by cdy on 23-10-2.
//

#ifndef TALON_RPC_FD_EVENT_GROUP_H
#define TALON_RPC_FD_EVENT_GROUP_H
#include "string"
#include "vector"
#include "fd_event.h"
#include "mutex.h"

namespace talon {
    class FdEventGroup {
    public:

    FdEventGroup(int size);

    ~FdEventGroup();
    Fd_Event* getFdEvent(int fd);

    public:
    static FdEventGroup* GetFdEventGroup();

    private:
    int m_size {0};
    std::vector<Fd_Event*> m_fd_group;
    Mutex m_mutex;

};


} // talon

#endif //TALON_RPC_FD_EVENT_GROUP_H
