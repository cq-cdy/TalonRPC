//
// Created by cdy on 23-10-3.
//

#include "tcp_client.h"

#include "cstring"
#include "err_code.h"
#include "eventloop.h"
#include "fd_event_group.h"
#include "log.h"
#include "memory"
#include "sys/socket.h"
#include "unistd.h"
talon::TcpClient::TcpClient(const talon::NetAddr::s_ptr &peer_addr) {
    /*
     * 这里 客户端有一个event_loop 并不是说客户端就成了服务端
     * event_loop只是起一个fd监听的作用
     *
     * */
    m_event_loop = Eventloop::GetCurrentEventLoop();
    m_peer_addr = peer_addr;
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
    if (m_fd < 0) {
        ERRORLOG("TcpClient::TcpClient() error failed to create fd")
        return;
    }
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();

    m_connection = std::make_shared<TcpConnection>(
            m_event_loop, m_fd, 128, m_peer_addr, nullptr, TcpType::TcpClientType);
    m_connection->setType(TcpClientType);
}

talon::TcpClient::~TcpClient() {
    if (m_fd < 0) {
        close(m_fd);
    }
}

/*
    因为在构造函数中把fd设置成了非阻塞的
    所以如果直接连接上就会返回0
    如果没有连接上就会返回-1，但是errno会被设置成EINPROGRESS
    如果是EINPROGRESS就说明正在连接
*/
void talon::TcpClient::connect(const std::function<void()> &done) {
    bool is_connected{false};
    int rt =
            ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if (rt == 0) {
        DEBUGLOG("connect [%s] sussess", m_peer_addr->toString().c_str());
        m_connection->setState(Connected);
        initLocalAddr();
        is_connected = true;
        if (done) {
            done();
        }
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // epoll监听可写事件，然后判断错误码
            m_fd_event->listen(Fd_Event::OUT_EVENT, [&is_connected,this, done]() {
                int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
                if ((rt < 0 && errno == EISCONN) || (rt == 0)) {
                    DEBUGLOG("connect [%s ] sussess", m_peer_addr->toString().c_str());
                    initLocalAddr();
                    m_connection->setState(Connected);
                    is_connected = true;
                } else {
                    if (errno == ECONNREFUSED) {
                        m_connect_errcode = ERROR_PEER_CLOSED;
                        m_connect_error_info = "connect refused, sys error = " + std::string(strerror(errno));
                    } else {
                        m_connect_errcode = ERROR_FAILED_CONNECT;
                        m_connect_error_info = "connect unkonwn error，sys error = " + std::string(strerror(errno));
                    }
                    ERRORLOG("connect errror,errno=%d，error=%s", errno, strerror(errno));
                    close(m_fd);
                    m_fd = socket(m_peer_addr->getFamily(), SOCK_STREAM, 0);
                }
                //连接完后需要去掉可写事件的监听，不然会一直触发
                m_event_loop->deleteEpollEvent(m_fd_event);

                DEBUGLOG("now begin to done");
                // 如果连接完成，才会执行回调函数
                if (done && is_connected){
                    done();
                }
            });
            m_event_loop->addEpollEvent(m_fd_event);

            if (!m_event_loop->isLooping()) {
                m_event_loop->loop();
            }
        } else {
            ERRORLOG("connect errror, errno=%d, error=%s", errno,
                     strerror(errno));
            m_connect_errcode = ERROR_FAILED_CONNECT;
            m_connect_error_info =
                    "connect error, sys error = " + std::string(strerror(errno));
            if (done) {
                done();
            }
        }
    }
}

/*
    用户调用此方法

    把用户需要传入的message 和 done函数
   通过m_connection->pushSendMessage(message,done); 传入到m_connection中，
   加入到m_write_dones中，然后设置写监听，当对端可写时
    就会调用m_connection->onWrite(),在onWrite中会判断如果当前类型是TcpClientType，那么就会
    调用m_coder->encode(messages,
   m_out_buffer);把message编码成字节流，然后把字节流写入到m_out_buffer中
    然后再调用m_connection->onWrite()，把m_out_buffer中的数据写入到传输给服务器，服务器那边就会调用
    m_connection->onRead()，然后在onRead()中调用m_coder->decode(m_in_buffer,
   messages);把字节流解码成message
    然后再调用用户传入的done函数，把message传给用户。
*/
void talon::TcpClient::writeMessage(
        const talon::AbstractProtocol::s_ptr &message,
        const std::function<void(AbstractProtocol::s_ptr)> &done) {
    m_connection->pushSendMessage(message, done);
    m_connection->listenWrite();  // 客户端/服务端 监听写
}

void talon::TcpClient::readMessage(
        const std::string &req_id,
        const std::function<void(AbstractProtocol::s_ptr)> &done) {
    m_connection->pushReadMessage(req_id, done);
    m_connection->listenRead();  // 客户端/服务端 监听读
}

void talon::TcpClient::stop() {
    if (m_event_loop->isLooping()) {
        m_event_loop->stop();
    }
}

int talon::TcpClient::getConnectErrorCode() const { return m_connect_errcode; }

std::string talon::TcpClient::getconnectErrorInfo() {
    return m_connect_error_info;
}

talon::NetAddr::s_ptr talon::TcpClient::getPeerAddr() { return m_peer_addr; }

talon::NetAddr::s_ptr talon::TcpClient::getLocalAddr() { return m_local_addr; }

void talon::TcpClient::initLocalAddr() {
    sockaddr_in local_addr{};
    socklen_t len = sizeof(local_addr);
    if ((getsockname(m_fd, reinterpret_cast<sockaddr *>(&local_addr), &len)) !=
        0) {
        ERRORLOG("initLocalAddr error, getsockname error. errno=%d，error=%s ",
                 errno, strerror(errno));
        return;
    }
    if (m_local_addr == nullptr) {
        m_local_addr = std::make_shared<IPNetAddr>(local_addr);
    }
}
