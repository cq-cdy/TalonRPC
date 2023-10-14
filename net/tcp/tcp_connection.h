//
// Created by cdy on 23-10-2.
//

#ifndef TALON_RPC_TCP_CONNECTION_H
#define TALON_RPC_TCP_CONNECTION_H

#include "memory"
#include "queue"
#include "net_addr.h"
#include "iothread.h"
#include "tcp_buffer.h"
#include "coder/abstract_protocol.h"
#include "coder/abstract_coder.h"

#include "rpc/rpc_dispatcher.h"

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
    class RpcDispatcher;
    class TcpConnection {

    public:
        typedef std::shared_ptr<TcpConnection> s_ptr;

        TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr,
                      TcpType type = TcpType::TcpServerType);

        TcpConnection(Eventloop *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr,
                      TcpType type = TcpType::TcpServerType);

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

        void setType(TcpType type) { m_type = type; }

        TcpType m_type{TcpServerType};
        std::string *m_res{nullptr};

        std::function<void(std::string &)> m_write_done{nullptr};
        std::function<void(std::string &)> m_read_done{nullptr};

        void pushSendMessage(const AbstractProtocol::s_ptr &message,
                             const std::function<void(AbstractProtocol::s_ptr)> &done);
        void pushReadMessage(const std::string& req_id,
                             const std::function<void(AbstractProtocol::s_ptr)>& done);

        NetAddr::s_ptr getLocalAddr();

        NetAddr::s_ptr getPeerAddr();

        void reply(std::vector<AbstractProtocol::s_ptr>& replay_messages);
    private:

        IOThread *m_io_thread{nullptr};
        Eventloop *m_event_loop{nullptr};
        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;
        TcpBuffer::s_ptr m_in_buffer;
        TcpBuffer::s_ptr m_out_buffer;
        int m_fd{0};
        TcpState m_state;

        Fd_Event *m_fd_event{nullptr};
        AbstractCoder *m_coder{nullptr};

        std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;
        std::map<std::string,std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;
    };

} // talon

#endif //TALON_RPC_TCP_CONNECTION_H
