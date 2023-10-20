add_executable(test_rpc_server test_rpc_server.cc order.pb.cc
        ../net/rpc/rpc_channel.cc
        ../net/rpc/rpc_channel.h
        ../net/rpc/msg_id_util.cc
        ../net/rpc/msg_id_util.h
        ../common/run_time.cc
        ../common/run_time.h
        ../net/rpc/rpc_interface.cc
        ../net/rpc/rpc_interface.h
        ../common/exception.h
        ../common/mutex.h)



add_executable(test_rpc_client test_rpc_client.cc order.pb.cc
        ../net/rpc/rpc_channel.cc
        ../net/rpc/rpc_channel.h
        ../net/rpc/msg_id_util.cc
        ../net/rpc/msg_id_util.h
        ../common/run_time.cc
        ../common/run_time.h
        ../net/rpc/rpc_interface.cc
        ../net/rpc/rpc_interface.h
        ../common/exception.h
        ../common/mutex.h
        ../net/rpc/service_discovery.h)



aux_source_directory(../common COMMON_LIST)
aux_source_directory(../net NET_LIST)
aux_source_directory(../net/tcp TCP_LIST)
aux_source_directory(../net/coder CODER_LIST)
aux_source_directory(../net/rpc RPC_LIST)

#target_sources(test_log PUBLIC ${COMMON_LIST} )
#target_sources(test_eventloop PUBLIC ${COMMON_LIST} ${NET_LIST} )
#target_sources(test_tcp PUBLIC ${COMMON_LIST} ${NET_LIST} ${TCP_LIST} ${CODER_LIST} ${RPC_LIST} )
#target_sources(test_client PUBLIC ${COMMON_LIST} ${NET_LIST}  ${TCP_LIST} ${CODER_LIST}  ${RPC_LIST} )
target_sources(test_rpc_server PUBLIC ${COMMON_LIST} ${NET_LIST}  ${TCP_LIST} ${CODER_LIST}  ${RPC_LIST} )
target_sources(test_rpc_client PUBLIC ${COMMON_LIST} ${NET_LIST}  ${TCP_LIST} ${CODER_LIST}  ${RPC_LIST} )
#
#target_link_libraries(test_log tinyxml pthread)
#target_link_libraries(test_eventloop tinyxml pthread)
#target_link_libraries(test_tcp tinyxml pthread protobuf)
#target_link_libraries(test_client tinyxml pthread protobuf)
target_link_libraries(test_rpc_server tinyxml pthread protobuf)
target_link_libraries(test_rpc_client tinyxml pthread protobuf)