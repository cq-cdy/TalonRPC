//
// Created by cdy on 23-9-27.
//

#ifndef TALONRPC_UTIL_H
#define TALONRPC_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace talon{

    pid_t get_pid();

    pid_t get_thread_id();

    int64_t get_now_ms();
}


#endif //TALONRPC_UTIL_H
