//
// Created by cdy on 23-11-4.
//

#ifndef TALON_RPC_HIGH_AVAILABILITY_H
#define TALON_RPC_HIGH_AVAILABILITY_H
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <functional>
#include <string>
#include "log.h"

static int current_start_count = 0;
static int child_pid = 0;
static void handle_sigchld(int sig) {
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {
        printf("hui shou\n");
    }
}
static void handle_sigint(int sig) {
    printf("in handle_sigint kill %d \n", child_pid);

    if (kill(child_pid, SIGKILL) < 0) {
        perror("kill err");
    } else {
        printf("kill succ\n");
    }
    exit(0);
}
class HighAvai {
private:
    HighAvai() = default;

public:
    static HighAvai* getInstance(std::function<void()> f) {
        static HighAvai h;
        h.server_func_ = f;
        return &h;
    }
    void setRestartCount(int count) { this->restart_count_ = count; }

    int start(int argc, char* argv[]) {
        char* program_name = argv[0];
        if (argc > 1) {
            current_start_count = std::atoi(argv[1]);
            INFOLOG("restart_count = %d, current_start_count= %d\n",
                    restart_count_, current_start_count);
            if (current_start_count > restart_count_) {
                printf("no restart\n");
                exit(2);
            }
        }
        // 安装SIGCHLD信号处理函数
        struct sigaction sa;
        sa.sa_handler = &handle_sigchld;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
        if (sigaction(SIGCHLD, &sa, 0) == -1) {
            perror("sigaction failed");
            exit(1);
        }
        pid_t pid = fork();  // 创建子进程
        printf("argc = %d\n", argc);
        if (pid == -1) {

            ERRORLOG("Failed to fork()");
            exit(-1);
        } else if (pid > 0) {  // fu
            child_pid = pid;
            DEBUGLOG("zi id : %d \n", child_pid);
            signal(SIGINT, handle_sigint);

            DEBUGLOG("current_start_count = %d --\n", current_start_count);

            int status;
            waitpid(pid, &status, 0);  // 等待子进程结束
            char str_restart_count[10];
            sprintf(str_restart_count, "%d", current_start_count + 1);
            char* new_argv[] = {program_name, str_restart_count, nullptr};
            execvp(program_name, new_argv);
            exit(1);
        } else {
            server_func_();
            exit(0);
        }
    }

private:
    std::function<void()> server_func_;
    int restart_count_ = 5;
};
#endif //TALON_RPC_HIGH_AVAILABILITY_H
