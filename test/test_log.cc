//
// Created by cdy on 23-9-29.
//

#include <pthread.h>
#include "log.h"
#include "config.h"


void* fun(void*) {

    int i = 20;
    while (i--) {
        DEBUGLOG("debug this is thread in %s", "fun");
        INFOLOG("info this is thread in %s", "fun");
    }

    return nullptr;
}

int main() {

    talon::Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();


    pthread_t thread;
    pthread_create(&thread, nullptr, &fun, nullptr);

    int i = 20;
    while (i--) {
        DEBUGLOG("test debug log %s", "11");
        INFOLOG("test info log %s", "11");
    }

    pthread_join(thread, nullptr);
    return 0;
}