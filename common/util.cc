//
// Created by cdy on 23-9-27.
//
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "util.h"


namespace talon {

    static int g_pid = 0; // process id

    static thread_local int t_thread_id = 0;

    pid_t get_pid() {

        if (g_pid != 0) {
            return g_pid;
        }
        return getpid();

    }

    pid_t get_thread_id() {
        if (t_thread_id != 0) {
            return t_thread_id;
        }
        return syscall(SYS_gettid);
    }

    int64_t get_now_ms() {
        timeval timeval{};
        gettimeofday(&timeval, nullptr);
        return timeval.tv_sec * 1000 + timeval.tv_usec / 1000;
    }
}

