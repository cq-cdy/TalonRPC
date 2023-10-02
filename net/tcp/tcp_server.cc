//
// Created by cdy on 23-10-2.
//

#include "tcp_server.h"

#include <utility>
#include "eventloop.h"
#include "tcp_connection.h"
#include "log.h"
namespace talon {

    TcpServer::TcpServer(NetAddr::s_ptr  local_addr):m_local_addr(std::move(local_addr)) {
        init();
        INFOLOG("rocket TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
    }
    TcpServer::~TcpServer() {
        if (m_main_event_loop) {
            delete m_main_event_loop;
            m_main_event_loop = nullptr;
        }
    }

    void TcpServer::start() {
        m_io_thread_group->start();
        m_main_event_loop->loop();
    }

    void TcpServer::init() {

        // 创建Accpet对象，就在main epoll 中监听连接的
        m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

        // 拿到 main线程 epoll  中的Eventloop 对象
        m_main_event_loop = Eventloop::GetCurrentEventLoop();
        m_io_thread_group = new IOThreadGroup(2);


        // 封装fd_event
        // epoll_event->data->ptr = ${this Fd_Event}
        m_listen_fd_event = new Fd_Event(m_acceptor->getListenFd());
        m_listen_fd_event->listen(Fd_Event::IN_EVENT, [this] { onAccept(); });


        //把 listen_fd 加到红黑树
        m_main_event_loop->addEpollEvent(m_listen_fd_event);

    }

    void TcpServer::onAccept() {

        // 拿到客户端的 cli_fd 和connection对象
        auto re = m_acceptor->accept();
        int client_fd = re.first;
        NetAddr::s_ptr peer_addr = re.second;

        m_client_counts++;

        // 把 cleintfd 添加到任意 IO 线程里面
        IOThread* io_thread = m_io_thread_group->getIOThread();

        //并将该连接在TcpConnection构造函数中封装成fd_event中 并挂载 确定子线程的epollz
        TcpConnection::s_ptr connetion = std::make_shared<TcpConnection>(io_thread, client_fd, 128, peer_addr);
        connetion->setState(Connected);

        m_client.insert(connetion); // 防止 connetion析构

        INFOLOG("TcpServer succ get client, fd=%d", client_fd);
    }


} // talon