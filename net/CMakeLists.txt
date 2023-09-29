#add_executable(${PROJECT_NAME} ${SRC_LIST1} ${SRC_LIST2}
#        fd_event.cc
#        fd_event.h
#        wakeup_fd_event.cc
#        wakeup_fd_event.h)
#
#
#aux_source_directory(. S1)
#aux_source_directory(../common S2)
#
#set(SRC_LIST ${S1} ${S2})
#target_sources(${PROJECT_NAME} PUBLIC ${SRC_LIST} )
#target_link_libraries(${PROJECT_NAME} tinyxml)