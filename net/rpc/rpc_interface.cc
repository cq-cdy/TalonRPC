//
// Created by cdy on 23-10-18.
//

#include "memory"
#include "log.h"
#include "rpc_closure.h"
#include "rpc_controller.h"
#include "rpc_interface.h"

namespace talon {
    RpcInterface::RpcInterface(const google::protobuf::Message* req, google::protobuf::Message* rsp, RpcClosure* done, RpcController* controller)
            : m_req_base(req), m_rsp_base(rsp), m_done(done) , m_controller(controller) {
        INFOLOG("RpcInterface");
    }

    RpcInterface::~RpcInterface() {
        INFOLOG("~RpcInterface");

        reply();

        destroy();

    }

    void RpcInterface::reply() {
        // reply to client
        // you should call is when you wan to set response back
        // it means this rpc method done
        if (m_done) {
            m_done->Run();
        }

    }

    std::shared_ptr<RpcClosure> RpcInterface::newRpcClosure(std::function<void()>& cb) {
        return std::make_shared<RpcClosure>(shared_from_this(), cb);
    }


    void RpcInterface::destroy() {
        if (m_req_base) {
            delete m_req_base;
            m_req_base = nullptr;
        }

        if (m_rsp_base) {
            delete m_rsp_base;
            m_rsp_base = nullptr;
        }

        if (m_done) {
            delete m_done;
            m_done = nullptr;
        }

        if (m_controller) {
            delete m_controller;
            m_controller = nullptr;
        }

    }
} // talon