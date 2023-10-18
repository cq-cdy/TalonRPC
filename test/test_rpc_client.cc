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

#include "order.pb.h"


void test_tcp_client() {

    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>("127.0.0.1", 12345);
    talon::TcpClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("conenct to [%s] success", addr->toString().c_str());
        std::shared_ptr<talon::TinyPBProtocol> message = std::make_shared<talon::TinyPBProtocol>();
        message->m_msg_id = "99998888";
        message->m_pb_data = "test pb data";

        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");

        if (!request.SerializeToString(&(message->m_pb_data))) {
            ERRORLOG("serilize error");
            return;
        }

        message->m_method_name = "Order.makeOrder";

        client.writeMessage(message, [request](talon::AbstractProtocol::s_ptr msg_ptr) {
            DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str());
        });


        client.readMessage("99998888", [](talon::AbstractProtocol::s_ptr msg_ptr) {
            std::shared_ptr<talon::TinyPBProtocol> message = std::dynamic_pointer_cast<talon::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id.c_str(), message->m_pb_data.c_str());
            makeOrderResponse response;

            if (!response.ParseFromString(message->m_pb_data)) {
                ERRORLOG("deserialize error");
                return;
            }
            DEBUGLOG("get response success, response[%s]", response.ShortDebugString().c_str());
        });
    });
}

void test_rpc_channel() {

    NEWRPCCHANNEL("127.0.0.1:12345", channel);

    // std::shared_ptr<makeOrderRequest> request = std::make_shared<makeOrderRequest>();

    NEWMESSAGE(makeOrderRequest, request);
    NEWMESSAGE(makeOrderResponse, response);

    request->set_price(100);
    request->set_goods("apple");

    NEWRPCCONTROLLER(controller);
    controller->SetMsgId("99998888");
    controller->SetTimeout(10000);

    std::shared_ptr<talon::RpcClosure> closure = std::make_shared<talon::RpcClosure>(nullptr, [request, response, channel, controller]() mutable {
        if (controller->GetErrorCode() == 0) {
            INFOLOG("call rpc success, request[%s], response[%s]", request->ShortDebugString().c_str(), response->ShortDebugString().c_str());
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
        // channel->getTcpClient()->stop();
        channel.reset();
    });

    {
        std::shared_ptr<talon::RpcChannel> channel = std::make_shared<talon::RpcChannel>(talon::RpcChannel::FindAddr("127.0.0.1:12345"));
        ;
        channel->Init(controller, request, response, closure);
        Order_Stub(channel.get()).makeOrder(controller.get(), request.get(), response.get(), closure.get());
    }

    // CALLRPRC("127.0.0.1:12345", Order_Stub, makeOrder, controller, request, response, closure);



    // xxx
    // 协程
}

int main() {

    talon::Config::SetGlobalConfig(nullptr);

    talon::Logger::InitGlobalLogger(0);

    // test_tcp_client();
    test_rpc_channel();

    INFOLOG("test_rpc_channel end");

    return 0;
}