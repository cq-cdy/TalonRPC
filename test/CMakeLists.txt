

#生成发布服务的可执行文件
add_executable(rpc_server test_rpc_server.cc order.pb.cc )

#生成客户端的可执行文件
add_executable(rpc_client test_rpc_client.cc order.pb.cc )

#生成 服务发现中间件的可执行文件
add_executable(rpc_service_discovery ../net/service_discovery/run_server_discovery.cc)
add_executable(test_interface ../net/service_discovery/test_run_time_sd_interface.cc)

target_link_libraries(rpc_server tinyxml pthread protobuf talon_rpc)
target_link_libraries(rpc_client tinyxml pthread protobuf talon_rpc)


