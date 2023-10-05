//
// Created by cdy on 23-10-4.
//

#ifndef TALON_RPC_TINYPB_PROTOCOL_H
#define TALON_RPC_TINYPB_PROTOCOL_H



#include "abstract_protocol.h"
namespace talon{
    class TinyPBProtocol:public AbstractProtocol{
    public:
        static char PB_START;
        static char PB_END;

        int32_t  m_pk_len{0};
        int32_t m_req_id_len{0};
        //req_id 继承自f


        int32_t m_method_len {0};
        std::string m_menthod_name;
        int32_t m_err_code {0};
        int32_t m_err_info_len{0};
        std::string m_err_info;
        std::string m_pb_data;
        int32_t m_check_sum;

    };
}
#endif //TALON_RPC_TINYPB_PROTOCOL_H
