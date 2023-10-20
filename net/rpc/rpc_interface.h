//
// Created by cdy on 23-10-18.
//

#ifndef TALON_RPC_RPC_INTERFACE_H
#define TALON_RPC_RPC_INTERFACE_H
#include "memory"
#include "google/protobuf/message.h"
#include "rpc/rpc_controller.h"
namespace talon {

    class RpcClosure;

/*
 * Rpc Interface Base Class
 * All interface should extend this abstract class
*/

    class RpcInterface : public std::enable_shared_from_this<RpcInterface> {
    public:

        RpcInterface(const google::protobuf::Message* req, google::protobuf::Message* rsp, RpcClosure* done, RpcController* controller);

        virtual ~RpcInterface();

        // reply to client
        void reply();

        // free resourse
        void destroy();

        // alloc a closure object which handle by this interface
        std::shared_ptr<RpcClosure> newRpcClosure(std::function<void()>& cb);

        // core business deal method
        virtual void run() = 0;

        // set error code and error into to response message
        virtual void setError(int code, const std::string& err_info) = 0;

    protected:

        const google::protobuf::Message* m_req_base {nullptr};

        google::protobuf::Message* m_rsp_base {nullptr};

        RpcClosure* m_done {nullptr};        // callback

        RpcController* m_controller {nullptr};


    };
} // talon

#endif //TALON_RPC_RPC_INTERFACE_H
