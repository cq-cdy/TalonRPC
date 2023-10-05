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


    TcpServer::TcpServer(NetAddr::s_ptr  local_addr):m_local_addr(std::move(local_addr)) {
        init();
        INFOLOG("talon TcpServer listen sucess on [%s]", m_local_addr->toString().c_str());
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
        m_io_thread_group = new IOThreadGroup(3);


        // 封装fd_event
        // epoll_event->data->ptr = ${this Fd_Event}
        m_listen_fd_event = new Fd_Event(m_acceptor->getListenFd());
        m_listen_fd_event->listen(Fd_Event::IN_EVENT, [this] { onAccept(); });

        //把 listen_fd 加到红黑树
        m_main_event_loop->addEpollEvent(m_listen_fd_event);

    }


    //异步回调函数，当有客户端来连接的时候，再去调用accept，和传统的socket不一样
    //传统的socket是accept函数一直监听
    void TcpServer::onAccept() {

        // 拿到客户端的 cli_fd 和connection对象
        auto re = m_acceptor->accept();
        int client_fd = re.first;
        NetAddr::s_ptr peer_addr = re.second;

        m_client_counts++;

        // 把 cleintfd 添加到任意 IO 线程里面
        IOThread* io_thread = m_io_thread_group->getIOThread();

       
        /*
        并将该连接在TcpConnection构造函数中封装成fd_event中 并挂在确定io_thread的epoll,
        connetion 对象存活在主线程的m_client中，在connetion构造函数中，通过io_thread
        获取到了确定的epoll，然后把fd_event挂载到了确定的epoll上，也就是说服务端的connection
        对象的 onRead和onWrite回调绑定，是发生在构造的时候的主线程的，传给epoll线程的也是一个
        把int fd（以为线程间的fd是共享的，因此fd能在多个线程间唯一标识一个连接）
         封装成一个的一个fd_event，绑定的函数地址，仍然是主线程的函数地址，也就是说，当fd触发
         都读事件时，就会在epoll线程中执行主线程的onRead函数，这样就实现了线程间的通信。
        */
        TcpConnection::s_ptr connetion = std::make_shared<TcpConnection>(io_thread, client_fd, 128, peer_addr,TcpType::TcpServerType);
        connetion->setState(Connected);

        m_client.insert(connetion); // 防止 connetion析构

        INFOLOG("TcpServer succ get client, fd=%d", client_fd);
    }


} // talon