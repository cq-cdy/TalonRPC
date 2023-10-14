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
void test_tcp_connect(){
    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>("127.0.0.1", 12345);
    talon::TcpClient client(addr);
    client.connect([&client,addr](){
        DEBUGLOG("test connect to [%s] success",addr->toString().c_str());
        std::shared_ptr message = std::make_shared<talon::TinyPBProtocol>();
        message->m_msg_id="99998888";
        message->m_pb_data = "test pb data";
        makeOrderRequest request;
        request.set_price(100);
        request.set_goods("apple");
        if(!request.SerializeToString(&(message->m_pb_data))){
            ERRORLOG("SERILIZE ERROR")
            return;
        }
        message->m_method_name = "Order.makeOrder";

        client.writeMessage(message, [request](const talon::AbstractProtocol::s_ptr& msg_ptr) {
            DEBUGLOG("send message success, request[%s]", request.ShortDebugString().c_str());
        });// 开启客户端的写监听->写完关闭监听
        client.readMessage("99998888",[](const talon::TinyPBProtocol::s_ptr& msg_ptr){
            std::shared_ptr message = std::dynamic_pointer_cast<talon::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("req_id[%d] get response %s",message->getReqId().c_str(),message->m_pb_data.c_str());
            makeOrderResponse response;
            if(!response.ParseFromString(message->m_pb_data)){
                ERRORLOG("DESERILIZE ERROR")
            }
            DEBUGLOG("get response success，response[%s]", response.ShortDebugString().c_str());
            DEBUGLOG("get response success，res_info[%s]", response.res_info().c_str());
            DEBUGLOG("get response success，order_id[%s]", response.order_id().c_str());
        });// 开启客户端的读监听，客户端不用关闭读的监听（用来接收服务端发来的消息）
    });
}
int main() {

    talon::Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();

    test_tcp_connect();
    return 0;
}