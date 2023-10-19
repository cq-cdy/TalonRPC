//
// Created by cdy on 23-10-2.
//

#include "tcp_server.h"

#include <utility>
#include "eventloop.h"
#include "tcp_connection.h"
#include "log.h"
namespace talon {

    /*
        1.TcpServer(开启主线程的event_loop,并将linsten_fd挂上树并绑定回调函数onAccpet()，
        开启多线程，每个线程一个event_loop，并且主线程可以拿到子线程的句柄)

        2.onAccpet()即有对端连接时被回调，创建TcpConnection对象
    
    
    */


    TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(std::move(local_addr)) {

        init();

        INFOLOG("talon TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
    }

    TcpServer::~TcpServer() {
        if (m_main_event_loop) {
            delete m_main_event_loop;
            m_main_event_loop = nullptr;
        }
        if (m_io_thread_group) {
            delete m_io_thread_group;
            m_io_thread_group = nullptr;
        }
        if (m_listen_fd_event) {
            delete m_listen_fd_event;
            m_listen_fd_event = nullptr;
        }
    }


    void TcpServer::init() {

        m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

        m_main_event_loop = Eventloop::GetCurrentEventLoop();
        m_io_thread_group = new IOThreadGroup(Config::GetGlobalConfig()->m_io_threads);

        m_listen_fd_event = new Fd_Event(m_acceptor->getListenFd());
        m_listen_fd_event->listen(Fd_Event::IN_EVENT, [this] { onAccept(); });

        m_main_event_loop->addEpollEvent(m_listen_fd_event);

        m_clear_client_timer_event = std::make_shared<TimerEvent>(5000, true, [this] { ClearClientTimerFunc(); });
        m_main_event_loop->addTimerEvent(m_clear_client_timer_event);

    }


    void TcpServer::onAccept() {
        // 阻塞等待连接
        auto re = m_acceptor->accept();
        int client_fd = re.first;
        NetAddr::s_ptr peer_addr = re.second;

        m_client_counts++;

        // 把 cleintfd 添加到任意 IO 线程里面
        IOThread* io_thread = m_io_thread_group->getIOThread();
        TcpConnection::s_ptr connetion = std::make_shared<TcpConnection>(io_thread->getEventLoop(), client_fd, 128, peer_addr, m_local_addr);
        connetion->setState(Connected);

        m_client.insert(connetion);

        INFOLOG("TcpServer succ get client, fd=%d", client_fd);
    }

    void TcpServer::start() {
        m_io_thread_group->start();
        m_main_event_loop->loop();
    }


    void TcpServer::ClearClientTimerFunc() {
        auto it = m_client.begin();
        for (it = m_client.begin(); it != m_client.end(); ) {
            // TcpConnection::ptr s_conn = i.second;
            // DebugLog << "state = " << s_conn->getState();
            if ((*it) != nullptr && (*it).use_count() > 0 && (*it)->getState() == Closed) {
                // need to delete TcpConnection
                DEBUGLOG("TcpConection [fd:%d] will delete, state=%d", (*it)->getFd(), (*it)->getState());
                it = m_client.erase(it);
            } else {
                it++;
            }

        }

    }


} // talon