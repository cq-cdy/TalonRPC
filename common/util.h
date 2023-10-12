//
// Created by cdy on 23-9-27.
//

#ifndef TALONRPC_UTIL_H
#define TALONRPC_UTIL_H

#include <sys/types.h>
#include <unistd.h>
#include "arpa/inet.h"
#include "cstring"
namespace talon{

    pid_t get_pid();

    pid_t get_thread_id();

    int64_t get_now_ms();

    int32_t getInt32FromNetByte(const char* buf);
}


#endif //TALONRPC_UTIL_H
