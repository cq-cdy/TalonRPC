//
// Created by cdy on 23-10-14.
//
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include "log.h"
#include "config.h"
#include "tcp/tcp_client.h"
#include "coder/string_coder.h"
#include "coder/abstract_protocol.h"
#include "coder/tinypb_protocol.h"
#include "coder/tinypb_coder.h"
#include "order.pb.h"
#include "google/protobuf/service.h"
#include "rpc/rpc_dispatcher.h"
#include "log.h"
#include "config.h"
#include "tcp/tcp_server.h"
#include "tcp/net_addr.h"
#include "iostream"
class OrderImpl : public Order {
public:
    void makeOrder(google::protobuf::RpcController* controller,
                   const ::makeOrderRequest* request,
                   ::makeOrderResponse* response,
                   ::google::protobuf::Closure* done) {
        APPDEBUGLOG("end sleep 5s");
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20230514");
        APPDEBUGLOG("call makeOrder success");
        if (done) {
            done->Run();
            delete done;
            done = nullptr;
        }
    }

};


int main(int argc, char* argv[]) {

//    if (argc != 2) {
//        printf("Start test_rpc_server error, argc not 2 \n");
//        printf("Start like this: \n");
//        printf("./test_rpc_server ../conf/talon.xml \n");
//        return 0;
//    }

    talon::Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();
    DEBUGLOG("TEST LOG..............")
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    talon::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>("127.0.0.1", talon::Config::GetGlobalConfig()->m_port);

    talon::TcpServer tcp_server(addr);

    tcp_server.start();

    return 0;
}