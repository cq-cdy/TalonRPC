//
// Created by cdy on 23-10-2.
//
#include "log.h"
#include "config.h"
#include "tcp/tcp_server.h"
#include "tcp/net_addr.h"
#include "iostream"
void test_tcp_server() {
    talon::IPNetAddr::s_ptr addr = std::make_shared<talon::IPNetAddr>("127.0.0.1", 12345);

    DEBUGLOG("create addr %s", addr->toString().c_str());

    talon::TcpServer tcp_server(addr);

    tcp_server.start();
}
int main() {

    talon::Config::SetGlobalConfig("../conf/talon.xml");

    talon::Logger::InitGlobalLogger();

    test_tcp_server();
}