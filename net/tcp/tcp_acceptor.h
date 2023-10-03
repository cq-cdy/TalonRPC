//
// Created by cdy on 23-10-1.
//

#ifndef TALON_RPC_TCP_ACCEPTOR_H
#define TALON_RPC_TCP_ACCEPTOR_H
#include "memory"
#include "net_addr.h"
namespace talon {

    class TcpAcceptor {
    public:
        typedef std::shared_ptr<TcpAcceptor> s_ptr;

        TcpAcceptor( const NetAddr::s_ptr& local_addr);

        ~TcpAcceptor();

        std::pair<int, NetAddr::s_ptr> accept() const;

        int getListenFd() const;

    private:
        NetAddr::s_ptr m_local_addr; // 服务端监听的地址，addr -> ip:port

        int m_family {-1};

        int m_listenfd {-1}; // 监听套接字

    };

} // talon

#endif //TALON_RPC_TCP_ACCEPTOR_H
