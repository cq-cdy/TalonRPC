//
// Created by cdy on 23-10-2.
//

#include "tcp_connection.h"

#include <utility>
#include "unistd.h"
#include "fd_event_group.h"
#include "log.h"
#include "coder/string_coder.h"
#include "coder/tinypb_coder.h"

/*
    想两端读写数据怎么办？
    往buffer写数据就行了，然后设置监听，和回调函数，
    当实践触发的时候自动执行回调函数，然后在回调函数中
    操作buffer，这样就实现了读写数据。

*/
namespace talon {
    /*
           epoll不是普遍意义上的服务器，而只是一个文件描述符事件监听epoll
           就行服务端，运行的时候，就会在一个进程上监听所连接的fd的读写事件，比如
           有一个客户端来连接了，那么就会在这个进程上监听这个客户端的读写事件，然后
           就做出响应，写回客户端。
           但客户端进程运行的时候TcpServer 类其实就是开一个epoll，来监听文件描述符的读写事件
           比如客户端进程和 服务端通信的文件描述符的事件，比如给服务器发个一消息，或者接收
           服务器的消息，这个时候就会在这个进程上监听这个文件描述符的读写事件，虽然都是在
           TcpServer和TcpClient代码中开启epoll时会标识双方各自持有的connection描述符是客户端还是服务端
           ，只是在服务端和客户端运行的时候就会走不同是逻辑
           通过 enum TcpType {
           TcpServerType = 1,
           TcpClientType = 2,
           };来确定当前运行的loop中的tcp_connection是在服务端还是客户端


            还需要注意的是一个服务端 开启的多线程中的每个线程都会有一个epoll，但文件描述符
            是共享的。
       */

      // 因为需要向某个已经在某个线程的epoll中确定的建立连接监听的文件描述符上进行读写，
      //所哟*io_thread就是这个文件描述符所在的epoll所在的线程
    TcpConnection::TcpConnection(IOThread *io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr,TcpType type )

            : m_io_thread(io_thread), m_peer_addr(std::move(peer_addr)), m_state(NotConnected), m_fd(fd),m_type(type) {
        /**
         * 为什么红黑树上挂了那么多connection事件，这么就能找到对应对象的的m_in_buffer呢？
         * 1.首先epoll红黑树上的是epoll_event 对象
         * 2.我们在Fd_Event::listen中epoll_event->data->ptr = this我们封装了fd_event对象
         * 3.然后通过fd_event可以找到我们绑定的每个对象的回调函数
         * 如是客户端消息事件，就会回到this->onRead or onWrite上
         * 如是客户端请求连接事件，就指向的是TcpAcceptor的 accept()方法，在各自的
         * 方法里能访问到给自的对象成员变量了。
         */
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
        m_fd_event->setNonBlock();
        m_event_loop = m_io_thread->getEventLoop();//把事件往这个线程的epoll上挂
        if(m_type == TcpType::TcpServerType){
            listenRead();
        }
        m_coder =new TinyPBCoder();

    }

    TcpConnection::TcpConnection(Eventloop *loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr,TcpType type)
            : m_event_loop(loop), m_peer_addr(std::move(peer_addr)), m_state(NotConnected), m_fd(fd),m_type(type) {

        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
        m_fd_event->setNonBlock();

        /*如果是在服务端新建的一个连接，那肯定是要
        监听客户端发来的消息，所以开启写读事件的监听*/
        if(m_type == TcpType::TcpServerType){
            listenRead();//这里的listenRead()就是在epoll红黑树上挂了一个读事件
        }
       // m_coder = new StringCoder();
        m_coder = new TinyPBCoder();
    }

    TcpConnection::~TcpConnection() {
        DEBUGLOG("~TcpConnection");
    }


    /*
    @details
    关于Server端，Client端 onRead()，excute()，onWrite()中的区别
    onRead():
            主要做的事情，就算从网络端读取字节流到buffer中
    excute():
            就是对buffer中的字节序列按业务操作
                作为Server端时，就是对buffer中的字节序列按照RPC协议解析，
            然后执行业务逻辑，然后把结果写到out_buffer中，然后注册写事件
            等可写时再把out_buffer中的结果写回去，然后关闭写事件监听。
                作为Client端时，首先肯定是客户端往m_write_dones中写入，然后序列化到
            out_buffer中写入后，然后开启写事件监听，等可写时再把out_buffer中的结果写回去，
            然后关闭写事件监听。
    onWrite()：
            主要做的事情，就是把out_buffer中的字节流写到网络端
            作为Client端时，就是把用户传入到m_write_dones中的数据序列化到out_buffer中的字节流。
            作为Server端时，就是把out_buffer中的字节流写到客户端

    因此顺序是:客户端->writeMessage->m_write_dones-> onWrite()-> excute()
                    ->服务器->onRead()-> excute()->onWrite()->客户端onRead()->m_read_dones
    */
    void TcpConnection::onRead() { // 读回调
        // 1. 从 socket 缓冲区，调用 系统的 read 函数读取字节 in_buffer 里面

        if (m_state != Connected) {
            ERRORLOG("onRead error, client has already disconneced, addr[%s], clientfd[%d]",
                     m_peer_addr->toString().c_str(), m_fd);
            return;
        }

        bool is_read_all = false;
        bool is_close = false;
        while (!is_read_all) {
            if (m_in_buffer->writeAble() == 0) {
                m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
            }
            int read_count = m_in_buffer->writeAble();
            int write_index = m_in_buffer->writeIndex();

            int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
            DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
            if (rt > 0) {
                m_in_buffer->moveWriteIndex(rt);
                if (rt == read_count) {
                    continue;
                } else if (rt < read_count) {
                    is_read_all = true;
                    break;
                }
            } else if (rt == 0) {
                is_close = true;
                break;
            } else if (rt == -1 && errno == EAGAIN) {
                is_read_all = true;
                break;
            }
        }

        if (is_close) {
            //TODO:
            INFOLOG("peer closed, peer addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
            clear();
            return;
        }

        if (!is_read_all) {
            ERRORLOG("not read all data");
        }
        // TODO: 简单的 echo, 后面补充 RPC 协议解析
        excute();

    }

    void TcpConnection::excute() {
        if (m_type == TcpServerType) {
//            // 将 RPC 请求执行业务逻辑，获取 RPC 响应, 再把 RPC 响应发送回去
//            std::vector<char> tmp;
//            int size = m_in_buffer->readAble();
//            tmp.resize(size);
//            m_in_buffer->readFromBuffer(tmp, size);
//
//            std::string msg;
//            for (char i: tmp) {
//                msg += i;
//            }
//
//            INFOLOG("success get request[%s] from client[%s]", msg.c_str(), m_peer_addr->toString().c_str());
//
//            m_out_buffer->writeToBuffer(msg.c_str(), msg.length());
            std::vector<AbstractProtocol::s_ptr> result;
            std::vector<AbstractProtocol::s_ptr> replay_messages;
            m_coder->decode(result,m_in_buffer);
            for(const auto& i: result){
                INFOLOG(" success get request[%s] from client[%s ]", i->m_msg_id.c_str(), m_peer_addr->toString().c_str());
                std::shared_ptr<TinyPBProtocol> message  = std::make_shared<TinyPBProtocol>();
                message->m_pb_data="hello. this is talon rpc test data";
                message->m_msg_id= i->m_msg_id;
                replay_messages.emplace_back(message);
            }

            m_coder->encode(replay_messages,m_out_buffer);
            listenWrite();

        } else { // 客户端
            std::vector<AbstractProtocol::s_ptr> result;
            m_coder ->decode(result,m_in_buffer);
            for(int i = 0 ;i < result.size();++i){
                std::string req_id = result[i]->getReqId();
                auto it = m_read_dones.find(req_id);

                if(it !=m_read_dones.end()){
                    auto func = it->second;
                    /*
                     * 异步编程中，某个对象可能会在回调函数中被使用。为了确保回调发生时对象仍然存在，
                     * 你可以使用 std::enable_shared_from_this 从当前对象获取一个 shared_ptr，
                     * 并确保其生命周期被正确管理。
                     */
                    func(result[i]->shared_from_this());
                }

            }
        }
    }

    void TcpConnection::listenWrite() {
        m_fd_event->listen(Fd_Event::OUT_EVENT, [this] { onWrite(); });
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::listenRead() {
        m_fd_event->listen(Fd_Event::IN_EVENT, [this] { onRead(); });
        m_event_loop->addEpollEvent(m_fd_event);
    }


    void TcpConnection::onWrite() {
        // 将当前 out_buffer 里面的数据全部发送给 client
        DEBUGLOG("ON onWrite");

        if (m_state != Connected) {
            ERRORLOG("onWrite error, client has already disconneced, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
            return;
        }

        if (m_type == TcpClientType) {
            //  1. 将 message encode 得到字节流
            // 2. 将字节流入到 buffer 里面，然后全部发送
            DEBUGLOG("client --------------------------------------------------")
            std::vector<AbstractProtocol::s_ptr> messages;

            for (auto & m_write_done_ : m_write_dones) {
                messages.push_back(m_write_done_.first);
            }
            m_coder->encode(messages, m_out_buffer); // 回写到m_out_buffer中
        }
        bool is_write_all = false;
        while(true) {
            if (m_out_buffer->readAble() == 0) {
                DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }
            int write_size = m_out_buffer->readAble();
            int read_index = m_out_buffer->readIndex();

            int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);

            if (rt >= write_size) {
                DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            } if (rt == -1 && errno == EAGAIN) {
                // 发送缓冲区已满，不能再发送了。
                // 这种情况我们等下次 fd 可写的时候再次发送数据即可
                ERRORLOG("write data error, errno==EAGIN and rt == -1");
                break;
            }
        }
        if (is_write_all) { // 写完之后，服务端就不再关心本次的写事件
            m_fd_event->cancle(Fd_Event::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
        }

        if (m_type == TcpClientType) {
            for (auto & m_w_done : m_write_dones) {
                m_w_done.second(m_w_done.first);
            }
            m_write_dones.clear();
        }
    }

    void TcpConnection::setState(const TcpState state) {
        m_state = state;
    }

    TcpState TcpConnection::getState() const {
        return m_state;
    }

    void TcpConnection::clear() {
        // 处理一些关闭连接后的清理动作
        if (m_state == Closed) {
            return;
        }
        m_event_loop->deleteEpollEvent(m_fd_event);

        m_state = Closed;

    }

    void TcpConnection::shutdown() {
        if (m_state == Closed || m_state == NotConnected) {
            return;
        }

        // 处于半关闭
        m_state = HalfClosing;

        // 调用 shutdown 关闭读写，意味着服务器不会再对这个 fd 进行读写操作了
        // 发送 FIN 报文， 触发了四次挥手的第一个阶段
        // 当 fd 发生可读事件，但是可读的数据为0，即 对端发送了 FIN
        ::shutdown(m_fd, SHUT_RDWR);

    }

    void
    TcpConnection::pushSendMessage(const AbstractProtocol::s_ptr& message,
                                   const std::function<void(AbstractProtocol::s_ptr)>& done) {
        m_write_dones.emplace_back(message, done);
    }

    void
    TcpConnection::pushReadMessage(const std::string& req_id, const std::function<void(AbstractProtocol::s_ptr)>&done) {
        m_read_dones.insert(std::make_pair(req_id,done));

    }




} // talon