//
// Created by cdy on 23-10-14.
//

#ifndef TALON_RPC_MSG_ID_UTIL_H
#define TALON_RPC_MSG_ID_UTIL_H

#include "string"

namespace talon {

    class MsgIDUtil {
    public:
        static std::string GenMsgID();
    };

} // talon

#endif //TALON_RPC_MSG_ID_UTIL_H
