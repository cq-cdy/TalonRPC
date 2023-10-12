//
// Created by cdy on 23-10-3.
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
void test_connect() {

    // 调用 conenct 连接 server
    // wirte 一个字符串
    // 等待 read 返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }

    sockaddr_in server_addr{};
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    if(rt < 0){
        DEBUGLOG("connect faild");
        return ;
    }
    DEBUGLOG("connect success");

    std::string msg = "hello rocket!";

    rt = write(fd, msg.c_str(), msg.length());

    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

    char buf[100];
    rt = read(fd, buf, 100);
    DEBUGLOG("rt = %d",rt);
    DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());

}
void test_tcp_connect(){
    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>("127.0.0.1", 12345);

    talon::TcpClient client(addr);
    client.connect([&client,addr](){
        DEBUGLOG("test connect to [%s] success",addr->toString().c_str());
        std::shared_ptr message = std::make_shared<talon::TinyPBProtocol>();
        message->m_msg_id="123456789";
        message->m_pb_data = "test pb data";
        client.writeMessage(message,[](const talon::TinyPBProtocol::s_ptr& msg_ptr){
            DEBUGLOG("send message success");
        }); // 开启客户端的写监听->写完关闭监听
        client.readMessage("123456789",[](const talon::TinyPBProtocol::s_ptr& msg_ptr){
            std::shared_ptr message = std::dynamic_pointer_cast<talon::TinyPBProtocol>(msg_ptr);
            DEBUGLOG("req_id[%d] get response %s",message->getReqId().c_str(),message->m_pb_data.c_str());
        });// 开启客户端的读监听，客户端不用关闭读的监听（用来接收服务端发来的消息）
    });
}
int main() {

    talon::Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();

   // test_connect();
   test_tcp_connect();
    return 0;
}