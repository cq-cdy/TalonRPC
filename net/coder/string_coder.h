//
// Created by cdy on 23-10-4.
//

#ifndef TALON_RPC_STRING_CODER_H
#define TALON_RPC_STRING_CODER_H

#include "abstract_protocol.h"
#include "abstract_coder.h"

namespace talon {


    class StringProtocol : public AbstractProtocol {

    public:
        std::string info;

    };

    class StringCoder : public AbstractCoder {
        // 将 message 对象转化为字节流，写入到 buffer
        void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr out_buffer) override {
            for (const auto &message: messages) {
                std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(message);
                out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
            }
        }

        // 将 buffer 里面的字节流转换为 message 对象
        void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer) override {
            std::vector<char> re;
            buffer->readFromBuffer(re, buffer->readAble());
            std::string info;
            for (char i: re) {
                info += i;
            }

            std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
            msg->info = info;
            msg->m_msg_id = "12345";
            out_messages.push_back(msg);
        }

    };

} // talon

#endif //TALON_RPC_STRING_CODER_H
