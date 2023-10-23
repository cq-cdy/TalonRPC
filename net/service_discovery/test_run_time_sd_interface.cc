//
// Created by cdy on 23-10-21.
//
#include "run_time_service_discovery_interface.h"
#include "iostream"
int main(){
    Runtime_Command command("127.0.0.1",9090);
//     std::cout <<command.add_service("Order.makeOrder","127.0.0.1",12345) .get_info() <<std::endl;
//    // std::cout <<command.delete_service("Order.queryOrder").get_info() <<std::endl;
    std::cout <<command.look_up() .get_info() <<std::endl;
}