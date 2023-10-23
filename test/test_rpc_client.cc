//
// Created by cdy on 23-10-14.
//
#include <unistd.h>

#include <memory>
#include <string>

#include "config.h"
#include "log.h"
#include "order.pb.h"
#include "rpc/rpc_channel.h"
#include "rpc/rpc_closure.h"
#include "rpc/rpc_controller.h"

void gerOrderID(int price,std::string goods){
    std::shared_ptr<talon::RpcChannel> channel =
            std::make_shared<talon::RpcChannel>(nullptr);
    std::shared_ptr<makeOrderRequest> request =
            std::make_shared<makeOrderRequest>();
    std::shared_ptr<makeOrderResponse> response =
            std::make_shared<makeOrderResponse>();
    std::shared_ptr<talon::RpcController> controller =
            std::make_shared<talon::RpcController>();

    /*业务逻辑*/
    request->set_price(100);
    request->set_goods(goods);

    std::shared_ptr<talon::RpcClosure> closure = std::make_shared<
            talon::RpcClosure>(nullptr, [request, response, channel,
            controller]() mutable {
        if (controller->GetErrorCode() == 0) {
            INFOLOG("!!call rpc success, request[%s], response[%s]",
                    request->ShortDebugString().c_str(),
                    response->ShortDebugString().c_str())
            /* 业务逻辑*/
            controller->SetMsgId("99998888");
            controller->SetTimeout(1000000000);
        } else {
            ERRORLOG(
                    "call rpc failed, request[%s], error code[%d], error info[%s]",
                    request->ShortDebugString().c_str(), controller->GetErrorCode(),
                    controller->GetErrorInfo().c_str());
        }
        INFOLOG("now exit eventloop");
        channel->getTcpClient()->stop();
        channel.reset();
    });

    /*submit your task*/
    channel->Init(controller, request, response, closure);
    Order_Stub stub(channel.get());
    stub.makeOrder(controller.get(), request.get(), response.get(),
                   closure.get());
}
int main() {
    talon::Config::setServiceCenterMap("../conf/service_center.conf");
    talon::Config::SetGlobalConfig(nullptr); // 客户端传null
    talon::Logger::InitGlobalLogger(0);     // 客户端不需要日志

    // user code
    gerOrderID(100,"apple");
    return 0;
}