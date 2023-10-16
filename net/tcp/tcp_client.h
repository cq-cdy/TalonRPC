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

        TcpClient(const NetAddr::s_ptr &peer_addr);

        ~TcpClient();

        //异步connect
        void connect(const std::function<void()> &done);

        void writeMessage(const AbstractProtocol::s_ptr &, const std::function<void(AbstractProtocol::s_ptr)> &);

        void readMessage(const std::string &req_id, const std::function<void(AbstractProtocol::s_ptr)> &done);

        void  stop();

        int getConnectErrorCode() const;
        std::string getconnectErrorInfo();
        NetAddr::s_ptr getPeerAddr();
        NetAddr::s_ptr  getLocalAddr();
        void initLocalAddr();
    private:

        NetAddr::s_ptr m_peer_addr;
        NetAddr::s_ptr m_local_addr;
        Eventloop *m_event_loop{nullptr};
        int m_fd{-1};
        Fd_Event *m_fd_event{nullptr};
        TcpConnection::s_ptr m_connection;

        int m_connect_errcode{0};
        std::string m_connect_error_info;
    };
}


#endif //TALON_RPC_TCP_CLIENT_H
