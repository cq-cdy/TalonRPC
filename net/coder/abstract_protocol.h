//
// Created by cdy on 23-10-3.
//

#ifndef TALON_RPC_ABSTRACT_PROTOCOL_H
#define TALON_RPC_ABSTRACT_PROTOCOL_H
#include "memory"
namespace talon{
    class AbstractProtocol{
    public:
        typedef  std::shared_ptr<AbstractProtocol> s_ptr;
    };
}
#endif //TALON_RPC_ABSTRACT_PROTOCOL_H
