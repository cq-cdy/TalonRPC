//
// Created by cdy on 23-10-13.
//

#ifndef TALON_RPC_RPC_CONTROLLER_H
#define TALON_RPC_RPC_CONTROLLER_H

#include "google/protobuf/service.h"
#include "log.h"
#include "tcp/net_addr.h"
namespace talon {

    class RpcController : public google::protobuf::RpcController {

    public:
        RpcController() { INFOLOG("RpcController"); }
        ~RpcController() override { INFOLOG("~RpcController"); }

        void Reset() override;

        bool Failed() const override;

        std::string ErrorText() const override;

        void StartCancel() override;

        void SetFailed(const std::string& reason) override;

        bool IsCanceled() const override;

        void NotifyOnCancel(google::protobuf::Closure* callback) override;

        void SetError(int32_t error_code, const std::string& error_info);

        int32_t GetErrorCode() const;

        std::string GetErrorInfo();

        void SetMsgId(const std::string& msg_id);

        std::string GetMsgId();

        void SetLocalAddr(NetAddr::s_ptr addr);

        void SetPeerAddr(NetAddr::s_ptr addr);

        NetAddr::s_ptr GetLocalAddr();

        NetAddr::s_ptr GetPeerAddr();

        void SetTimeout(int timeout);

        int GetTimeout() const;

        bool Finished() const;

        void SetFinished(bool value);

    private:
        int32_t m_error_code {0};
        std::string m_error_info;
        std::string m_msg_id;

        bool m_is_failed {false};
        bool m_is_cancled {false};
        bool m_is_finished {false};

        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;

        int m_timeout {1000};   // ms

    };


} // talon

#endif //TALON_RPC_RPC_CONTROLLER_H
