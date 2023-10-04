//
// Created by cdy on 23-10-4.
//

#ifndef TALON_RPC_ABSTRACT_CODER_H
#define TALON_RPC_ABSTRACT_CODER_H
#include "abstract_protocol.h"
#include "tcp/tcp_buffer.h"
#include "vector"
namespace talon{

    class AbstractCoder {
    public:

        // 将 message 对象转化为字节流，写入到 buffer
        virtual void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) = 0;

        // 将 buffer 里面的字节流转换为 message 对象
        virtual void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) = 0;


    };
}
#endif //TALON_RPC_ABSTRACT_CODER_H
