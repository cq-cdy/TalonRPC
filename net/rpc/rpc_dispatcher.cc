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
#include "run_time.h"
#include "rpc_closure.h"
#include "service_discovery/run_time_service_discovery_interface.h"

namespace talon {
#define DELETE_RESOURCE(XX) \
  if (XX != NULL) { \
    delete XX;      \
    XX = NULL;      \
  }                 \

    static RpcDispatcher *g_rpc_dispatcher = nullptr;

    RpcDispatcher *RpcDispatcher::GetRpcDispatcher() {
        if (g_rpc_dispatcher != nullptr) {
            return g_rpc_dispatcher;
        }
        g_rpc_dispatcher = new RpcDispatcher;
        return g_rpc_dispatcher;
    }

    // 主要是 服务端在执行方法时的准备设置工作，最后一句话才是执行调用的方法
    // 虽然 写了一个 dispatch 类，其实就说在tcpconnection拿到数据之后
    // 就实例化 一个 request 和 response，然后CallMethod 
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
            setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
            return;
        }

        // 根据service_name找到对应的service
        auto it = m_service_map.find(service_name);
        if (it == m_service_map.end()) {
            ERRORLOG("%s | sericve neame[%s] not found", req_protocol->m_msg_id.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
            return;
        }
        service_s_ptr service = (*it).second;

        // 根据method_name找到对应的MethodDescriptor
        const google::protobuf::MethodDescriptor *method = service->GetDescriptor()->FindMethodByName(method_name);
        if (method == nullptr) {
            ERRORLOG("%s | method neame[%s] not found in service[%s]", req_protocol->m_msg_id.c_str(),
                     method_name.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "method not found");
            return;
        }

        // request message 用父类指针指向，实例化的是一个protobuf中自己写的request对象
        google::protobuf::Message *req_msg = service->GetRequestPrototype(method).New();

        // 反序列化，将 pb_data 反序列化为 req_msg
        if (!req_msg->ParseFromString(req_protocol->m_pb_data)) {
            ERRORLOG("%s | deserilize error", req_protocol->m_msg_id.c_str(), method_name.c_str(),
                     service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");
            DELETE_RESOURCE(req_msg);
            return;
        }

        INFOLOG("%s | get rpc request[%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());


        // 实例化一个response message 用父类指针指向，实例化的是一个protobuf中自己写的response对象
        google::protobuf::Message *rsp_msg = service->GetResponsePrototype(method).New();

        // 实例化一个rpc controller
        auto *rpc_controller = new RpcController();
        rpc_controller->SetLocalAddr(connection->getLocalAddr());
        rpc_controller->SetPeerAddr(connection->getPeerAddr());
        rpc_controller->SetMsgId(req_protocol->m_msg_id);

        RunTime::GetRunTime()->m_msgid = req_protocol->m_msg_id;
        RunTime::GetRunTime()->m_method_name = method_name;

        //最终会在发布的业务函数里面被执行
        auto *closure = new RpcClosure(nullptr,
                                       [req_msg, rsp_msg, req_protocol, rsp_protocol, connection, rpc_controller, this]() mutable {
                                           DEBUGLOG("NOW  in RpcClosure -----------------")
                                           if (!rsp_msg->SerializeToString(&(rsp_protocol->m_pb_data))) {
                                               ERRORLOG("%s | serilize error, origin message [%s]",
                                                        req_protocol->m_msg_id.c_str(),
                                                        rsp_msg->ShortDebugString().c_str());
                                               setTinyPBError(rsp_protocol, ERROR_FAILED_SERIALIZE, "serilize error");
                                           } else {
                                               rsp_protocol->m_err_code = 0;
                                               rsp_protocol->m_err_info = "";
                                               INFOLOG("%s | dispatch success, requesut[%s], response[%s]",
                                                       req_protocol->m_msg_id.c_str(),
                                                       req_msg->ShortDebugString().c_str(),
                                                       rsp_msg->ShortDebugString().c_str());
                                           }

                                           std::vector<AbstractProtocol::s_ptr> replay_messages;
                                           replay_messages.emplace_back(rsp_protocol);
                                           connection->reply(replay_messages); // 监听写

                                       });
        DEBUGLOG("NOW  in before    service->CallMethod -----------------")
        // 这里服务端这里的CallMethod 接收的是自己定义的子类指针const ::makeOrderRequest* request,
        //                   ::makeOrderResponse* response,
        service->CallMethod(method, rpc_controller, req_msg, rsp_msg, closure);
    }


    bool RpcDispatcher::parseServiceFullName(const std::string &full_name, std::string &service_name,
                                             std::string &method_name) {
        if (full_name.empty()) {
            ERRORLOG("full name empty");
            return false;
        }
        size_t i = full_name.find_first_of('.');
        if (i == std::string::npos) {
            ERRORLOG("not find . in full name [%s]", full_name.c_str());
            return false;
        }
        service_name = full_name.substr(0, i);
        method_name = full_name.substr(i + 1, full_name.length() - i - 1);

        INFOLOG("parse sericve_name[%s] and method_name[%s] from full name [%s]", service_name.c_str(),
                method_name.c_str(), full_name.c_str());

        return true;

    }

    void RpcDispatcher::registerService(const service_s_ptr &service) {

        // local  registe
        std::string service_name = service->GetDescriptor()->full_name();

        m_service_map[service_name] = service;

        // remote  registe
        std:: string remote_ip = talon::Config::getServiceCenterMap()["serivce_center_ip"];
        int control_port = std::atoi(Config::getServiceCenterMap()["control_port"].c_str());
        Runtime_Command command(remote_ip, control_port);

        std::string local_ip = talon::Config::GetGlobalConfig()->m_local_ip;
        int local_port = talon::Config::GetGlobalConfig()->m_port;
        for (int i = 0; i < service->GetDescriptor()->method_count(); ++i) {
            const google::protobuf::MethodDescriptor *methodDescriptor = service->GetDescriptor()->method(i);
            std::string all_name;
            all_name += service_name;
            all_name += '.';
            all_name += methodDescriptor->name();
            auto res = command.add_service(all_name, local_ip, local_port);
            if (!res.get_is_success()) {
                ERRORLOG("error with [%s] to register service discovery center : [%s]", all_name.c_str(),
                         res.get_info().c_str());
            } else {

                INFOLOG("success [%s] register service discovery center : [%s:%d]", all_name.c_str(), local_ip.c_str(),
                        local_port);
            }
        }

    }

    void RpcDispatcher::setTinyPBError(const std::shared_ptr<TinyPBProtocol> &msg, int32_t err_code,
                                       const std::string &err_info) {
        msg->m_err_code = err_code;
        msg->m_err_info = err_info;
        msg->m_err_info_len = err_info.length();
    }

} // talon