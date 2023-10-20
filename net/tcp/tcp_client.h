//
// Created by cdy on 23-10-3.
//

#ifndef TALON_RPC_TCP_CLIENT_H
#define TALON_RPC_TCP_CLIENT_H

#include "net_addr.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "coder/abstract_protocol.h"

namespace talon {
    class TcpClient {
    public:
        typedef std::shared_ptr<TcpClient> s_ptr;

        TcpClient(const NetAddr::s_ptr& peer_addr);

        ~TcpClient();

        // 异步的进行 conenct
        // 如果 connect 完成，done 会被执行
        void connect(const std::function<void()>& done);

        // 异步的发送 message
        // 如果发送 message 成功，会调用 done 函数， 函数的入参就是 message 对象
        void writeMessage(const AbstractProtocol::s_ptr& message, std::function<void(AbstractProtocol::s_ptr)> done);


        // 异步的读取 message
        // 如果读取 message 成功，会调用 done 函数， 函数的入参就是 message 对象
        void readMessage(const std::string& msg_id, std::function<void(AbstractProtocol::s_ptr)> done);

        void stop();

        int getConnectErrorCode() const;

        std::string getConnectErrorInfo();

        NetAddr::s_ptr getPeerAddr();

        NetAddr::s_ptr getLocalAddr();

        void initLocalAddr();

        void addTimerEvent(const TimerEvent::s_ptr& timer_event);


    private:
        NetAddr::s_ptr m_peer_addr;
        NetAddr::s_ptr m_local_addr;

        Eventloop* m_event_loop {nullptr};

        int m_fd {-1};
        Fd_Event* m_fd_event {nullptr};

        TcpConnection::s_ptr m_connection;

        int m_connect_error_code {0};
        std::string m_connect_error_info;

    };
}


#endif //TALON_RPC_TCP_CLIENT_H