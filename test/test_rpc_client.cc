//
// Created by cdy on 23-10-14.
//
#include <unistd.h>
#include <memory>
#include <string>

#include "coder/abstract_protocol.h"
#include "coder/string_coder.h"
#include "coder/tinypb_coder.h"
#include "coder/tinypb_protocol.h"
#include "config.h"
#include "log.h"
#include "order.pb.h"
#include "rpc/rpc_closure.h"
#include "rpc/rpc_channel.h"
#include "rpc/rpc_controller.h"
#include "tcp/tcp_client.h"
#include "order.pb.h"

bool checkRpcFuncName(std::string rpc_funcName){
    return true;

}
void test_rpc_channel(std::string rpc_funcName) {

    std::shared_ptr<talon::RpcChannel> channel = std::make_shared<talon::RpcChannel>(
            talon::RpcChannel::FindAddr("127.0.0.1:12345"));
    std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
    std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();
    
    request->set_price(100);
    request->set_goods("apple");

    std::shared_ptr<talon::RpcController> controller = std::make_shared<talon::RpcController>();
    controller->SetMsgId("99998888");
    controller->SetTimeout(10000);

    std::shared_ptr<talon::RpcClosure> closure = std::make_shared<talon::RpcClosure>(nullptr, [request, response, channel, controller]() mutable {
        if (controller->GetErrorCode() == 0) {
            INFOLOG("!!call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
            // 执行业务逻辑
            if (response->order_id() == "xxx") {
                // xx
            }
        } else {
            ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]",
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
//    {
//        std::shared_ptr<talon::RpcChannel> channel_ = std::make_shared<talon::RpcChannel>(talon::RpcChannel::FindAddr("127.0.0.1:12345"));
//
//        channel_->Init(controller, request, response, closure);
//
//        Order_Stub stub(channel_.get());
//
//        printf("func name %s ----------------\n", stub.GetDescriptor()->name().c_str());
//        // 通过channel写的时候就注册一下读回调
//
//        stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
//    }
    // CALLRPRC("127.0.0.1:12345", Order_Stub, makeOrder, controller, request, response, closure);
    // xxx
    // 协程
}
void test_rpc_channel2(std::string rpc_funcName) {

    std::shared_ptr<talon::RpcChannel> channel = std::make_shared<talon::RpcChannel>(
            talon::RpcChannel::FindAddr("127.0.0.1:12346"));
    std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();
    std::shared_ptr<makeOrderResponse> response = std::make_shared<makeOrderResponse>();



    std::shared_ptr<talon::RpcController> controller = std::make_shared<talon::RpcController>();

    std::shared_ptr<talon::RpcClosure> closure = std::make_shared<talon::RpcClosure>(nullptr, [request, response, channel, controller]() mutable {
        if (controller->GetErrorCode() == 0) {
            INFOLOG("!!call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
            // 执行业务逻辑

        } else {
            ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]",
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
    stub.queryOrder(controller.get(), request.get(), response.get(), closure.get());
}

int main() {

    talon::Config::SetGlobalConfig(nullptr);

    talon::Logger::InitGlobalLogger(0);

    //test_tcp_client();
    //test_rpc_channel("1");
    test_rpc_channel("1");
    INFOLOG("------------------------------------------")
    INFOLOG("test_rpc_channel end");

    return 0;
}