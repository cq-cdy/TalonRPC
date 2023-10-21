//
// Created by cdy on 23-10-14.
//

#include <unistd.h>

#include <memory>
#include <string>


#include "config.h"
#include "google/protobuf/service.h"
#include "log.h"
#include "order.pb.h"
#include "rpc/rpc_dispatcher.h"
#include "tcp/net_addr.h"
#include "tcp/tcp_server.h"
class OrderImpl : public Order {
   public:
    void makeOrder(google::protobuf::RpcController* controller,
                   const ::makeOrderRequest* request,
                   ::makeOrderResponse* response,
                   ::google::protobuf::Closure* done) {
        /************* 业务逻辑****************/
        if (request->price() < 10) {
            response->set_ret_code(-1);
            response->set_res_info("short balance");
            return;
        }
        response->set_order_id("20231015");
        APPDEBUGLOG("call makeOrder success");
        if (done) {
            done->Run();
            delete done;
            done = nullptr;
        }
        DEBUGLOG("NOW  in makeOrder -----------------")
    }

    void queryOrder(google::protobuf::RpcController* controller,
                    const ::makeOrderRequest* request,
                    ::makeOrderResponse* response,
                    ::google::protobuf::Closure* done) {
        if (done) {
            done->Run();
            delete done;
            done = nullptr;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Start test_rpc_server error, argc not 2 \n");
        printf("Start like this: \n");
        printf("./test_rpc_server ../conf/talon.xml \n");
        return 0;
    }

    talon::Config::SetGlobalConfig(argv[1]);

    talon::Logger::InitGlobalLogger();
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    talon::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    printf("port = %d\n", talon::Config::GetGlobalConfig()->m_port);
    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>(
        "127.0.0.1", talon::Config::GetGlobalConfig()->m_port);

    talon::TcpServer tcp_server(addr);

    tcp_server.start();

    return 0;
}