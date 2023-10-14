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
    void makeOrder(google::protobuf::RpcController *controller,
                   const ::makeOrderRequest *request,
                   ::makeOrderResponse *response,
                   ::google::protobuf::Closure *done) {
            if(request->price() < 0){
                response->set_ret_code(-1);
                response->set_res_info("short balance");
                return;
            }
            response->set_order_id("20231014");
    }

};

void test_tcp_server() {
    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>("127.0.0.1", 12345);

    DEBUGLOG("create addr %s", addr->toString().c_str());

    talon::TcpServer tcp_server(addr);

    tcp_server.start();
}

int main() {

    talon::Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();
    auto service = std::make_shared<OrderImpl>();

    talon::RpcDispatcher::GetRpcDispatcher()->registerService(service);
    test_tcp_server();
    return 0;
}