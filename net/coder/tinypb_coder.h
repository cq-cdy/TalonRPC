//
// Created by cdy on 23-10-4.
//

#ifndef TALON_RPC_TINYPB_CODER_H
#define TALON_RPC_TINYPB_CODER_H
#include "abstract_coder.h"
#include <memory>
#include <vector>
#include "tinypb_protocol.h"
namespace talon {

    class TinyPBCoder: public AbstractCoder {

    public:
        TinyPBCoder();
        void encode(std::vector<AbstractProtocol::s_ptr> &messages, TcpBuffer::s_ptr out_buffer) override;

        void decode(std::vector<AbstractProtocol::s_ptr> &out_messages, TcpBuffer::s_ptr buffer) override;
    private:
        const char *encodeTinyPB(const std::shared_ptr<TinyPBProtocol>& message, int &len);

    };

} // talon

#endif //TALON_RPC_TINYPB_CODER_H
