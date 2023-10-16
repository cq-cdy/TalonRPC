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

void test_tcp_connect() {
    talon::IPNetAddr::s_ptr addr =
            std::make_shared<talon::IPNetAddr>("127.0.0.1", 12345);
    talon::TcpClient client(addr);
    client.connect([&client, addr]() {
        DEBUGLOG("test connect to [%s] success", addr->toString().c_str());
        std::shared_ptr message = std::make_shared<talon::TinyPBProtocol>();
        message->m_msg_id = "99998888";
        message->m_pb_data = "test pb data";
        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");
        if (!request.SerializeToString(&(message->m_pb_data))) {
            ERRORLOG("SERILIZE ERROR")
            return;
        }
        message->m_method_name = "Order.makeOrder";

        client.writeMessage(
                message, [request](const talon::AbstractProtocol::s_ptr &msg_ptr) {
                    DEBUGLOG("send message success, request[%s]",
                             request.ShortDebugString().c_str());
                });  // 开启客户端的写监听->写完关闭监听

        client.readMessage("99998888", [](const talon::TinyPBProtocol::s_ptr &
        msg_ptr) {
            std::shared_ptr message =
                    std::dynamic_pointer_cast<talon::TinyPBProtocol>(msg_ptr);

            makeOrderResponse response;
            if (!response.ParseFromString(message->m_pb_data)) {
                ERRORLOG("DESERILIZE ERROR")
            }
            DEBUGLOG("req_id[%d] get response %s", message->getReqId().c_str(),
                     message->m_pb_data.c_str());
            DEBUGLOG("get response success，response[%s]",
                     response.ShortDebugString().c_str());
            DEBUGLOG("get response success，res_info[%s]",
                     response.res_info().c_str());
            DEBUGLOG("get response success，order_id[%s]",
                     response.order_id().c_str());
        });  // 开启客户端的读监听，客户端不用关闭读的监听（用来接收服务端发来的消息）
    });
}

void test_rpc_channel() {
    talon::IPNetAddr::s_ptr addr =
            std::make_shared<talon::IPNetAddr>("127.0.0.1", 12345);

    std::shared_ptr<talon::RpcChannel> channel =
            std::make_shared<talon::RpcChannel>(addr);

    std::shared_ptr<makeOrderRequest> request =
            std::make_shared<makeOrderRequest>();
    request->set_price(100);
    request->set_goods("apple");
    std::shared_ptr<makeOrderResponse> response =
            std::make_shared<makeOrderResponse>();
    std::shared_ptr<talon::RpcController> controller =
            std::make_shared<talon::RpcController>();
    controller->SetMsgId("99998888");
    std::shared_ptr<talon::RpcClosure> closure = std::make_shared<talon::RpcClosure>([&]() {
        INFOLOG("call rpc success,request = %s  ,response =  %s", request->DebugString().c_str(),
                response->DebugString().c_str())
        channel->getTcpClient()->stop();
        channel.reset();
    });
    channel->Init(controller, request, response, closure);
    Order_Stub stub(channel.get());

    stub.makeOrder(controller.get(), request.get(), response.get(), closure.get());
    DEBUGLOG("-------------------------------end line in test_rpc_channel() ")
}

int main() {
    talon::Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();

    //test_tcp_connect();
    test_rpc_channel();
    return 0;
}