//
// Created by cdy on 23-10-12.
//

#ifndef TALON_RPC_RPC_DISPATCHER_H
#define TALON_RPC_RPC_DISPATCHER_H
#include "memory"
#include "map"
#include "google/protobuf/service.h"
#include "coder/abstract_protocol.h"
#include "coder/tinypb_protocol.h"
#include "tcp/tcp_connection.h"
namespace talon {
    class TcpConnection;
    class RpcDispatcher {
    public:
        static RpcDispatcher* GetRpcDispatcher();
        typedef std::shared_ptr<google::protobuf::Service> service_s_ptr;
        void dispatch(const AbstractProtocol::s_ptr& request,const AbstractProtocol::s_ptr& response ,talon::TcpConnection*);
        void registerService(service_s_ptr service);
        void setTinyPBError(const std::shared_ptr<TinyPBProtocol>& msg,int32_t err_code,const std::string& err_info);
    private:
        bool parseServiceFullName(const std::string & full_name,std::string& service_name,std::string& method_name);

    private:
            std::map<std::string,service_s_ptr> m_service_map;
    };

} // talon

#endif //TALON_RPC_RPC_DISPATCHER_H
