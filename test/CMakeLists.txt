
add_executable(test_eventloop   test_eventloop.cc
        ../net/tcp/tcp_buffer.cc
        ../net/tcp/tcp_buffer.h
        ../net/tcp/net_addr.cc
        ../net/tcp/net_addr.h
        ../net/tcp/tcp_acceptor.cc
        ../net/tcp/tcp_acceptor.h)

add_executable(test_log  test_log.cc)
add_executable(test_tcp test_tcp.cc
        ../net/tcp/tcp_server.cc
        ../net/tcp/tcp_server.h
        ../net/tcp/tcp_connection.cc
        ../net/tcp/tcp_connection.h)


aux_source_directory(../common COMMON_LIST)
aux_source_directory(../net NET_LIST)
aux_source_directory(../net/tcp TCP_LIST)

target_sources(test_log PUBLIC ${COMMON_LIST} )
target_sources(test_eventloop PUBLIC ${COMMON_LIST} ${NET_LIST} )
target_sources(test_tcp PUBLIC ${COMMON_LIST} ${NET_LIST} ${TCP_LIST} )

target_link_libraries(test_log tinyxml pthread)
target_link_libraries(test_eventloop tinyxml pthread)
target_link_libraries(test_tcp tinyxml pthread)