//
// Created by cdy on 23-10-18.
//

#ifndef TALON_RPC_RUN_TIME_H
#define TALON_RPC_RUN_TIME_H
#include "string"
namespace talon {

    class RpcInterface;

    class RunTime {
    public:
        RpcInterface* getRpcInterface() const;

    public:
        static RunTime* GetRunTime();


    public:
        std::string m_msgid;
        std::string m_method_name;
        RpcInterface* m_rpc_interface {nullptr};

    };

} // talon

#endif //TALON_RPC_RUN_TIME_H
