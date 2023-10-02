//
// Created by cdy on 23-10-2.
//

#ifndef TALON_RPC_TCP_CONNECTION_H
#define TALON_RPC_TCP_CONNECTION_H

#include "memory"
#include "net_addr.h"
#include "iothread.h"
#include "tcp_buffer.h"

namespace talon {

    enum TcpState {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3,
        Closed = 4,
    };
    enum TcpType {
        TcpServerType = 1,
        TcpClientType = 2,
    };

    class TcpConnection {

    public:
        typedef std::shared_ptr<TcpConnection> s_ptr;

        TcpConnection(IOThread *io_thread, int fd, int buffer_size, const NetAddr::s_ptr& peer_addr);

        TcpConnection(Eventloop *io_thread, int fd, int buffer_size, NetAddr::s_ptr  peer_addr);

        ~TcpConnection();

        void onRead();
        void excute();
        void onWrite();
        void clear();
        void setState(TcpState state);

        TcpState getState() const;
        // 服务器主动关闭连接
        void shutdown();

        void listenWrite();

        void listenRead();

        void setType(TcpType type) { m_type = type;}
    private:

        IOThread* m_io_thread {nullptr};
        Eventloop* m_event_loop {nullptr};
        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;
    public:
        TcpBuffer::s_ptr m_in_buffer;
        TcpBuffer::s_ptr m_out_buffer;
        TcpState m_state;
        int m_fd {0};
        Fd_Event* m_fd_event {nullptr};
        TcpType m_type {TcpServerType};
        std::string* m_res {nullptr};

        std::function<void(std::string&)> m_write_done{nullptr};
        std::function<void(std::string&)> m_read_done{nullptr};
    };

} // talon

#endif //TALON_RPC_TCP_CONNECTION_H
