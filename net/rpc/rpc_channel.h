//
// Created by cdy on 23-10-14.
//

#ifndef TALON_RPC_RPC_CHANNEL_H
#define TALON_RPC_RPC_CHANNEL_H

#include "google/protobuf/service.h"
#include "tcp/net_addr.h"
#include "tcp/tcp_client.h"

namespace talon {

class RpcChannel : public google::protobuf::RpcChannel,
                   public std::enable_shared_from_this<RpcChannel> {
   public:
    typedef std::shared_ptr<RpcChannel> s_ptr;
    typedef std::shared_ptr<google::protobuf::RpcController> controller_s_ptr;
    typedef std::shared_ptr<google::protobuf::Message> message_s_ptr;
    typedef std::shared_ptr<google::protobuf::Closure> closure_s_ptr;

    RpcChannel(NetAddr::s_ptr peer_addr);
    ~RpcChannel() override;

    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller,
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *done) override;

    void Init(controller_s_ptr controller, message_s_ptr request,
              message_s_ptr response, closure_s_ptr done);

    google ::protobuf ::RpcController *getcontroller();
    google ::protobuf ::Message *getRequest();
    google ::protobuf ::Message *getResponse();
    google ::protobuf ::Closure *getclosure();

    TcpClient* getTcpClient();
   private:
    NetAddr::s_ptr m_peer_addr{nullptr};
    NetAddr::s_ptr m_local_addr{nullptr};
    controller_s_ptr m_controller{nullptr};
    message_s_ptr m_request{nullptr};
    message_s_ptr m_response{nullptr};
    closure_s_ptr m_done{nullptr};
    bool m_is_init{false};

    TcpClient::s_ptr m_client{nullptr};

};

}  // namespace talon

#endif  // TALON_RPC_RPC_CHANNEL_H
