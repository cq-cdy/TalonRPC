//
// Created by cdy on 23-10-14.
//

#include "rpc_channel.h"

#include <utility>

#include "coder/abstract_protocol.h"
#include "coder/tinypb_protocol.h"
#include "err_code.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "google/protobuf/service.h"
#include "log.h"
#include "msg_id_util.h"
#include "rpc_controller.h"
#include "tcp/tcp_client.h"

namespace talon {
    RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr)
            : m_peer_addr(std::move(peer_addr)) {
        m_client = std::make_shared<TcpClient>(m_peer_addr);
    }

    RpcChannel::~RpcChannel() {

        DEBUGLOG("in ~RpcChannel()-------------------- ")
    }

    void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                                google::protobuf::RpcController *controller,
                                const google::protobuf::Message *request,
                                google::protobuf::Message *response,
                                google::protobuf::Closure *done) {

        std::shared_ptr<TinyPBProtocol> req_protocol =
                std::make_shared<TinyPBProtocol>();

        auto my_controller = dynamic_cast<talon::RpcController *>(controller);
        if (my_controller == nullptr) {
            ERRORLOG("failed callmethod,RpcController convert error")
            return;
        }

        if (!m_is_init) {
            ERRORLOG("RpcChannel not init");
            return;
        }
        if (my_controller->GetMsgId().empty()) {
            req_protocol->m_msg_id = MsgIDUtil::GenMsgID();
            my_controller->SetMsgId(req_protocol->m_msg_id);
        } else {
            req_protocol->m_msg_id = my_controller->GetMsgId();
        }

        req_protocol->m_method_name = method->full_name();
        INFOLOG("%s|call method name [%s]", req_protocol->m_msg_id.c_str(),
                req_protocol->m_method_name.c_str());
        if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
            std::string err_info = "failed to serialize";
            my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
            ERRORLOG("%s | %s ,origin requeset[%s] ",
                     req_protocol->m_msg_id.c_str(), err_info.c_str(),
                     request->ShortDebugString().c_str());
            return;
        }

        /*
         * 因为RpcChannel 继承了std::enable_shared_from_this<RpcChannel>
         * 所以在类外用户创建 RpcChannel 的时候，需要使用
         * std::make_shared<RpcChannel>()，然后创建完
         * 对象之后，在对象内部，使用shared_from_this()获取
         * 指向类外创建的对象的引用计数，相当于引用计数+1，当用户
         * 类外创建的对象被销毁的时候，保证对象内部的引用计数不为0，
         * 不被销毁，因为下面connect函数是异步执行的，不知道什么是会
         * 销毁对象，所以需要保证对象不被销毁，所以需要使用shared_from_this()
         * 保证channel在回调函数被调用的时候，channel对象还存在。
         *
         */
        s_ptr channel = shared_from_this();
        m_client->connect([req_protocol, channel]() mutable {
            auto my_controller = dynamic_cast<talon::RpcController *>(
                    channel->getcontroller());
            if (channel->getTcpClient()->getConnectErrorCode() != 0) {
                my_controller->SetError(channel->getTcpClient()->getConnectErrorCode(),
                                        channel->getTcpClient()->getconnectErrorInfo());
                ERRORLOG("%s | connect error,error coode[%d],error info[%s]",
                         req_protocol->m_msg_id.c_str(), my_controller->GetErrorCode(),
                         my_controller->GetErrorInfo().c_str());
            }

            channel->getTcpClient()->writeMessage(req_protocol,
                                                  [req_protocol, channel](
                                                          const talon::TinyPBProtocol::s_ptr &msg_ptr) mutable {
                                                      INFOLOG("%s| send rpc request success. call method name[%s]",
                                                              req_protocol->m_msg_id.c_str(),
                                                              req_protocol->m_method_name.c_str());
                                                  });  // 开启客户端的写监听->写完关闭监听

            channel->getTcpClient()->readMessage(req_protocol->m_msg_id, [channel, my_controller](
                    const talon::AbstractProtocol::s_ptr &msg) mutable {
                std::shared_ptr<talon::TinyPBProtocol> rsp_protocol =
                        std::dynamic_pointer_cast<talon::TinyPBProtocol>(msg);
                if (rsp_protocol == nullptr) {
                    ERRORLOG("failed to dynamic_pointer_cast");
                    return;
                }
                INFOLOG(
                        "%s| success get rpc response success. call method "
                        "name[%s]",
                        rsp_protocol->m_msg_id.c_str(),
                        rsp_protocol->m_method_name.c_str());

                if (my_controller == nullptr) {
                    ERRORLOG("failed callmethod,RpcController convert error")
                    return;
                }
                if (!(channel->getResponse()->ParseFromString(
                        rsp_protocol->m_pb_data))) {
                    std::string err_info = "failed to deserialize";
                    my_controller->SetError(ERROR_FAILED_DESERIALIZE, err_info);
                    ERRORLOG("%s serialize erroe ",
                             rsp_protocol->m_msg_id.c_str())
                    return;
                }
                DEBUGLOG("rsp_protocol->m_pb_data =  %s ", rsp_protocol->m_pb_data.c_str());
                if (rsp_protocol->m_err_code != 0) {
                    ERRORLOG(
                            "%s | call rpc methood[%s] failed,error code[%d],error "
                            "info[%s]",
                            rsp_protocol->m_msg_id.c_str(),
                            rsp_protocol->m_method_name.c_str(),
                            rsp_protocol->m_err_code,
                            rsp_protocol->m_err_info.c_str());

                    my_controller->SetError(rsp_protocol->m_err_code,
                                            rsp_protocol->m_err_info);
                    return;
                }

                INFOLOG("%s | call rpc success,call method name[%s], peer addr[%s], local addr[%s]",
                        rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
                        channel->getTcpClient()->getPeerAddr()->toString().c_str(),
                        channel->getTcpClient()->getLocalAddr()->toString().c_str())

                if (channel->getclosure()) {
                    channel->getclosure()->Run();
                }

                channel.reset();

            });  // 开启客户端的读监听，
        });
    }

    void RpcChannel::Init(RpcChannel::controller_s_ptr controller,
                          RpcChannel::message_s_ptr request,
                          RpcChannel::message_s_ptr response,
                          RpcChannel::closure_s_ptr done
    ) {
        if (m_is_init) {
            return;
        }
        m_controller = std::move(controller);
        m_request = std::move(request);
        m_response = std::move(response);
        m_done = std::move(done);
        m_is_init = true;
    }

    google::protobuf::Message *RpcChannel::getRequest() { return m_request.get(); }

    google::protobuf::RpcController *RpcChannel::getcontroller() {
        return m_controller.get();
    }

    google::protobuf::Message *RpcChannel::getResponse() {
        return m_response.get();
    }

    google::protobuf::Closure *RpcChannel::getclosure() { return m_done.get(); }

    TcpClient *RpcChannel::getTcpClient() {
        return m_client.get();
    }

}  // namespace talon