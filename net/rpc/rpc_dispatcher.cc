//
// Created by cdy on 23-10-12.
//
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "rpc_dispatcher.h"
#include "log.h"
#include "coder/tinypb_protocol.h"
#include "err_code.h"
#include "rpc_controller.h"
#include "tcp/tcp_connection.h"

namespace talon {
    void RpcDispatcher::dispatch(const AbstractProtocol::s_ptr &request, const AbstractProtocol::s_ptr &response,
                                 talon::TcpConnection *connection) {
        std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
        std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);

        std::string method_full_name = req_protocol->m_method_name;
        std::string service_name;
        std::string method_name;
        rsp_protocol->m_msg_id = req_protocol->m_msg_id;
        rsp_protocol->m_method_name = req_protocol->m_method_name;

        if (!parseServiceFullName(method_full_name, service_name, method_name)) {
            setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NANE, "parse service name error");
            return;
        }
        //std::shared_ptr<google::protobuf::Service>
        // 启动服务的时候传入的，里面是OrderImpl（继承google::protobuf::Service）的实现的对象
        auto it = m_service_map.find(service_name);
        if (it == m_service_map.end()) {
            ERRORLOG("%s sericve neame[%s] not found", req_protocol->m_msg_id.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
            return;

        }
        service_s_ptr service = (*it).second;
        const google::protobuf::MethodDescriptor *method =
                service->GetDescriptor()->FindMethodByName(method_name);
        if (method == nullptr) {
            ERRORLOG("%s method neame[%s] not found in service[%s ]",
                     req_protocol->m_msg_id.c_str(), method_name.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "method not found");
            return;
        }

        google::protobuf::Message *req_msg = service->GetRequestPrototype(method).New();
        if (!req_msg->ParseFromString(req_protocol->m_pb_data)) {
            ERRORLOG("%s |deserilize error", req_protocol->m_msg_id.c_str(), method_name.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
            return;

        }
        INFOLOG("req_id[%s], get rpc request[%s]",
                req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());

        google::protobuf::Message *rsp_msg = service->GetResponsePrototype(method).New();
        auto *rpc_controller = new RpcController();
        rpc_controller->SetLocalAddr(connection->getLocalAddr());
        rpc_controller->SetPeerAddr(connection->getPeerAddr());
        rpc_controller->SetMsgId(req_protocol->m_msg_id);

        //真正执行的rpc 方法
        service->CallMethod(method, rpc_controller, req_msg, rsp_msg, nullptr);


        if (!rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data))) {
            ERRORLOG("%s | serilize error,origin message [s ]",
                     req_protocol->m_msg_id.c_str(), rsp_msg->ShortDebugString().c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "serilize error");
            return;

        }
        rsp_protocol->m_err_code = 0;

        INFOLOG("%s | dispatch success, requesut[%s] ,response[%s]",
                req_protocol->m_msg_id.c_str(),
                req_msg->ShortDebugString().c_str(),
                rsp_msg->ShortDebugString().c_str());

    }

    bool RpcDispatcher::parseServiceFullName(const std::string &full_name, std::string &service_name,
                                             std::string &method_name) {
        if (full_name.empty()) {
            ERRORLOG("full name empty");
            return false;
        }
        auto i = full_name.find_first_of('.');
        if (i == std::string::npos) {
            ERRORLOG("not find. in full name [%s]", full_name.c_str());
            return false;
        }
        service_name = full_name.substr(0, i);
        method_name = full_name.substr(i + 1, full_name.length() - i - 1);
        INFOLOG("parse sericve_name[%s] and method_name[%s] from full name [%s]", service_name.c_str(),
                method_name.c_str(), full_name.c_str());
        return true;

    }

    void
    RpcDispatcher::setTinyPBError(const std::shared_ptr<TinyPBProtocol> &msg, int32_t err_code,
                                  const std::string &err_info) {
        msg->m_err_code = err_code;
        msg->m_err_info = err_info;
        msg->m_err_info_len = err_info.length();
    }

    static RpcDispatcher *g_rpc_dispatcher = new RpcDispatcher();;

    RpcDispatcher *RpcDispatcher::GetRpcDispatcher() {
        return g_rpc_dispatcher;
    }

    void RpcDispatcher::registerService(const RpcDispatcher::service_s_ptr service) {
        auto name = service->GetDescriptor()->full_name();
        m_service_map[name] = service;
    }

} // talon