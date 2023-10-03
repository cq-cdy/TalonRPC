//
// Created by cdy on 23-10-3.
//

#ifndef TALON_RPC_TCP_CLIENT_H
#define TALON_RPC_TCP_CLIENT_H
#include "net_addr.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "coder/abstract_protocol.h"
namespace talon{
    class TcpClient {

    public:
        TcpClient(const NetAddr::s_ptr& peer_addr);
        ~TcpClient();

        //异步connect
        void connect(std::function<void()> done);

        void writeMessage(AbstractProtocol::s_ptr,std::function<void(AbstractProtocol::s_ptr)>);
        void readMessage(AbstractProtocol::s_ptr,std::function<void(AbstractProtocol::s_ptr)>);

    private:

        NetAddr::s_ptr m_peer_addr;
        Eventloop* m_event_loop {nullptr};
        int m_fd {-1};
        Fd_Event* m_fd_event{nullptr};
        TcpConnection::s_ptr m_connection;
    };
}



#endif //TALON_RPC_TCP_CLIENT_H
