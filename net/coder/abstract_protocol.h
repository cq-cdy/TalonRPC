//
//
// Created by cdy on 23-10-3.
//

#ifndef TALON_RPC_ABSTRACT_PROTOCOL_H
#define TALON_RPC_ABSTRACT_PROTOCOL_H

#include "memory"
#include "string"
namespace talon {
class AbstractProtocol :public std::enable_shared_from_this<AbstractProtocol>{
    public:
        typedef std::shared_ptr<AbstractProtocol> s_ptr;
        std::string getReqId() const{
            return m_msg_id;
        }

        void setReqId(const std::string& req_id){
            m_msg_id = req_id;
        }
        virtual ~AbstractProtocol() {}

    public:
        std::string m_msg_id;     // 请求号，唯一标识一个请求或者响应

    };

}
#endif //TALON_RPC_ABSTRACT_PROTOCOL_H
