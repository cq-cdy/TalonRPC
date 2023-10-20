//
// Created by cdy on 23-10-15.
//

#ifndef TALON_RPC_RPC_CLOSURE_H
#define TALON_RPC_RPC_CLOSURE_H
#include <utility>

#include "functional"
#include "exception.h"
#include "google/protobuf/stubs/callback.h"
#include "run_time.h"
#include "rpc_interface.h"
namespace talon {
    class RpcClosure : public google::protobuf::Closure {
    public:
        typedef std::shared_ptr<RpcInterface> it_s_ptr;

        RpcClosure(it_s_ptr interface, std::function<void()> cb) : m_rpc_interface(std::move(interface)), m_cb(std::move(cb)) {
            INFOLOG("RpcClosure");
        }

        ~RpcClosure() override {
            INFOLOG("~RpcClosure");
        }

        void Run() override {

            // 更新 runtime 的 RpcInterFace, 这里在执行 cb 的时候，都会以 RpcInterface 找到对应的接口，实现打印 app 日志等
            if (!m_rpc_interface) {
                RunTime::GetRunTime()->m_rpc_interface = m_rpc_interface.get();
            }

            try {
                if (m_cb != nullptr) {
                    m_cb(); // 用户重写的closure函数
                }
                if (m_rpc_interface) {
                    m_rpc_interface.reset();
                }
            } catch (TalonException& e) {
                ERRORLOG("talonException exception[%s], deal handle", e.what());
                e.handle();
                if (m_rpc_interface) {
                    m_rpc_interface->setError(e.errorCode(), e.errorInfo());
                    m_rpc_interface.reset();
                }
            } catch (std::exception& e) {
                ERRORLOG("std::exception[%s]", e.what());
                if (m_rpc_interface) {
                    m_rpc_interface->setError(-1, "unkonwn std::exception");
                    m_rpc_interface.reset();
                }
            } catch (...) {
                ERRORLOG("Unkonwn exception");
                if (m_rpc_interface) {
                    m_rpc_interface->setError(-1, "unkonwn exception");
                    m_rpc_interface.reset();
                }
            }

        }

    private:
        it_s_ptr m_rpc_interface {nullptr};
        std::function<void()> m_cb {nullptr};

    };

}  // namespace talon

#endif  // TALON_RPC_RPC_CLOSURE_H
