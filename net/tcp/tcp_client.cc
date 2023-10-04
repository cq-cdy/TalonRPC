//
// Created by cdy on 23-10-3.
//

#include "tcp_client.h"
#include "log.h"
#include "eventloop.h"
#include "fd_event_group.h"
#include "unistd.h"
#include "sys/socket.h"
#include "cstring"
talon::TcpClient::TcpClient(const talon::NetAddr::s_ptr& peer_addr) {

    /*
     * 这里 客户端有一个event_loop 并不是说客户端就成了服务端
     * event_loop只是起一个fd监听的作用
     *
     * */
    m_event_loop = Eventloop::GetCurrentEventLoop();
    m_peer_addr = peer_addr;
    m_fd = socket(peer_addr->getFamily(),SOCK_STREAM,0);
    if (m_fd < 0 ){
        ERRORLOG("TcpClient::TcpClient() error failed to create fd")
        return;
    }
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();

    m_connection =std::make_shared<TcpConnection>(m_event_loop,m_fd,128,m_peer_addr,TcpType::TcpClientType);
    m_connection->setType(TcpClientType);
}

talon::TcpClient::~TcpClient() {
    if(m_fd <0){
        close(m_fd);
    }
}

void talon::TcpClient::connect(const std::function<void()>& done) {
    int rt = ::connect(m_fd,m_peer_addr->getSockAddr(),m_peer_addr->getSockLen());
    /*
        因为在构造函数中把fd设置成了非阻塞的
        所以如果直接连接上就会返回0
        如果没有连接上就会返回-1，但是errno会被设置成EINPROGRESS
        如果是EINPROGRESS就说明正在连接
    */
    
    if (rt == 0){
        /*
         * 如果直接连接上就执行done
         * */
        DEBUGLOG("connect [%s] success",m_peer_addr->toString().c_str());
        if(done){done();}
    }else if(rt == -1){
        if(errno == EINPROGRESS){
            DEBUGLOG("EINPROGRESS");
            /*
                如果是正在连接，就需要持续监听
                socket的连接是否完成，于是用本地的event_loop
                来监听这个fd的可写事件，如果可写就说明连接成功。
            */
            m_fd_event->listen(Fd_Event::OUT_EVENT,[this,done](){
                int error  = 0;
                socklen_t error_len = sizeof(error) ;
                ::getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&error,&error_len);

                bool  is_connect_success = false;
                if(error == 0){
                    is_connect_success  = true;
                    m_connection->setState(Connected);
                    DEBUGLOG("connect [%s] success",m_peer_addr->toString().c_str());

                }else{
                    ERRORLOG("connect error errno = %d,error = %s",error,strerror(error));
                }
                m_fd_event->cancle(Fd_Event::OUT_EVENT);
                m_event_loop->addEpollEvent(m_fd_event);
                if(is_connect_success && done){
                    done();

                }
            });

            m_event_loop->addEpollEvent(m_fd_event);
            if(!m_event_loop->isLooping()){
                /*又因为设置了非阻塞，所以如果一直没连接上，就会很快的无线循环（设置的超时事件mei'yonm）*/
                m_event_loop->loop();
            }
        }else{
            ERRORLOG("connect error=%d ,error = %s",errno, strerror(errno));
            m_event_loop->stop();
        }
    }
}

void talon::TcpClient::writeMessage(const talon::AbstractProtocol::s_ptr& message,
                                    const std::function<void(AbstractProtocol::s_ptr)>& done) {
    m_connection->pushSendMessage(message,done);
    m_connection->listenWrite(); //客户端/服务端 监听写
}

void talon::TcpClient::readMessage(const std::string &req_id,
                                   const std::function<void(AbstractProtocol::s_ptr)>& done) {
    m_connection->pushReadMessage(req_id,done);
    m_connection->listenRead(); // 客户端/服务端 监听读

}

