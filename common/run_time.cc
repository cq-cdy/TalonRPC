//
// Created by cdy on 23-10-18.
//

#include "run_time.h"

namespace talon {
    thread_local RunTime* t_run_time = nullptr;

    RunTime* RunTime::GetRunTime() {
        if (t_run_time) {
            return t_run_time;
        }
        t_run_time = new RunTime();
        return t_run_time;
    }


    RpcInterface* RunTime::getRpcInterface() const {
        return m_rpc_interface;
    }

} // talon