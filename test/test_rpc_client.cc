//
// Created by cdy on 23-10-14.
//
#include <unistd.h>
#include <memory>
#include <string>
#include "config.h"
#include "log.h"
#include "order.pb.h"
#include "rpc/rpc_closure.h"
#include "rpc/rpc_channel.h"
#include "rpc/rpc_controller.h"

void test_rpc_channel() {

    std::shared_ptr<talon::RpcChannel> channel = std::make_shared<talon::RpcChannel>(
            nullptr);
    std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
    std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();

    request->set_price(100);
    request->set_goods("apple");

    std::shared_ptr<talon::RpcController> controller = std::make_shared<talon::RpcController>();
    controller->SetMsgId("99998888");
    controller->SetTimeout(10000);

    std::shared_ptr<talon::RpcClosure> closure = std::make_shared<talon::RpcClosure>
            (nullptr,
                                                                                     [request, response, channel, controller]() mutable {
                                                                                         if (controller->GetErrorCode() ==
                                                                                             0) {
                                                                                             INFOLOG("!!call rpc success, request[%s], response[%s]",
                                                                                                     request->ShortDebugString().c_str(),
                                                                                                     response->ShortDebugString().c_str());
                                                                                             if (response->order_id() ==
                                                                                                 "xxx") {
                                                                                                 // xx
                                                                                             }
                                                                                         } else {
                                                                                             ERRORLOG(
                                                                                                     "call rpc failed, request[%s], error code[%d], error info[%s]",
                                                                                                     request->ShortDebugString().c_str(),
                                                                                                     controller->GetErrorCode(),
                                                                                                     controller->GetErrorInfo().c_str());
                                                                                         }
                                                                                         INFOLOG("now exit eventloop");
                                                                                         channel->getTcpClient()->stop();
                                                                                         channel.reset();
                                                                                     });
    channel->Init(controller, request, response, closure);
    Order_Stub stub(channel.get());
    stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
}
int main() {
    talon::Config::SetGlobalConfig(nullptr);
    talon::Logger::InitGlobalLogger(0);
    test_rpc_channel();
    INFOLOG("------------------------------------------")
    INFOLOG("test_rpc_channel end");

    return 0;
}