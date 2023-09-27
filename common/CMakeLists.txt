set(SRC_LIST util.cc config.cc
        log.cc
        log.h
        ../net/eventloop.cc
        ../net/eventloop.h)
set(LIB_PATH /usr/lib/libtinyxml.a)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${LIB_PATH})



