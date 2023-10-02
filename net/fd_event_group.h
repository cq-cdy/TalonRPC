//
// Created by cdy on 23-10-2.
//

#ifndef TALON_RPC_FD_EVENT_GROUP_H
#define TALON_RPC_FD_EVENT_GROUP_H
#include "string"
#include "vector"
#include "fd_event.h"
#include "mutex"

namespace talon {

    class FdEventGroup  {
    public:

        ~FdEventGroup();
        Fd_Event* getFdEvent(int fd);
        static FdEventGroup* GetFdEventGroup();
    private:
        FdEventGroup(int size);
    private:
        int m_size{0};
        std::vector<Fd_Event*> m_fd_group;
        std::mutex m_mutex;

    };



} // talon

#endif //TALON_RPC_FD_EVENT_GROUP_H
