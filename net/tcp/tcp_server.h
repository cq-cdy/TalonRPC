//
// Created by cdy on 23-10-2.
//

#ifndef TALON_RPC_TCP_SERVER_H
#define TALON_RPC_TCP_SERVER_H

#include "set"
#include "tcp_acceptor.h"
#include "eventloop.h"
#include "iothreadgroup.h"
#include "net_addr.h"
#include "tcp_connection.h"
namespace talon {

    class TcpServer {
    public:
        TcpServer(NetAddr::s_ptr local_addr);

        ~TcpServer();

        void start();


    private:
        void init();

        // 当有新客户端连接之后需要执行
        void onAccept();

        // 清除 closed 的连接
        void ClearClientTimerFunc();


    private:
        TcpAcceptor::s_ptr m_acceptor;

        NetAddr::s_ptr m_local_addr;    // 本地监听地址

        Eventloop* m_main_event_loop {nullptr};    // mainReactor

        IOThreadGroup* m_io_thread_group {nullptr};   // subReactor 组

        Fd_Event* m_listen_fd_event;

        int m_client_counts {0};

        std::set<TcpConnection::s_ptr> m_client;

        TimerEvent::s_ptr m_clear_client_timer_event;

    };
} // talon

#endif //TALON_RPC_TCP_SERVER_H
