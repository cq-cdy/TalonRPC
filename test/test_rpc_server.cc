//
// Created by cdy on 23-10-14.
//

#include <memory>
#include <string>

#include "config.h"
#include "google/protobuf/service.h"
#include "high_availability.h"
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
    talon::Config::SetGlobalConfig("../conf/talon.xml");
    talon::Config::setServiceCenterMap("../conf/service_center.conf");
    talon::Logger::InitGlobalLogger();
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    talon::RpcDispatcher::GetRpcDispatcher()->registerService(service);
    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>(
            talon::Config::GetGlobalConfig()->m_local_ip,
            talon::Config::GetGlobalConfig()->m_port);
    printf("local_ip= %s,port = %d\n",
           talon::Config::GetGlobalConfig()->m_local_ip.c_str(),
           talon::Config::GetGlobalConfig()->m_port);

    auto start_service_func = [&]() {
        talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>(
                talon::Config::GetGlobalConfig()->m_local_ip,
                talon::Config::GetGlobalConfig()->m_port);
        talon::TcpServer tcp_server(addr);
        tcp_server.start();
    };

    // if you need high_availability service ,then like this :
            HighAvai* high_avai = HighAvai::getInstance(start_service_func);
            high_avai->setRestartCount(10/* defalut count = 5；*/);
            high_avai->start(argc,argv);

   //    if you  don't need ,just like this :
    //       start_service_func();

    return 0;
}