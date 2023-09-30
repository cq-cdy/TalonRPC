//
// Created by cdy on 23-9-30.
//

#ifndef TALON_RPC_IOTHREADGROUP_H
#define TALON_RPC_IOTHREADGROUP_H

#include "vector"
#include "log.h"
#include "iothread.h"

namespace talon {
    class IOThreadGroup {
    public:
        IOThreadGroup(int);

        ~IOThreadGroup();

        void start();

        void join();

        IOThread *getIOThread();

        std::vector<IOThread *> &getIOThread_group();
    private:

        int m_size{0};
        std::vector<IOThread *> m_io_thread_groups;

        int m_index{0};

    };

}


#endif //TALON_RPC_IOTHREADGROUP_H
