//
// Created by cdy on 23-10-15.
//

#ifndef TALON_RPC_RPC_CLOSURE_H
#define TALON_RPC_RPC_CLOSURE_H
#include <utility>

#include "functional"
#include "google/protobuf/stubs/callback.h"
namespace talon {
class RpcClosure : public google::protobuf::Closure {
   public:
    RpcClosure(std::function<void()> cb) : m_cb(std::move(cb)) {}
    virtual ~RpcClosure() = default;
    void Run() override {
        if (m_cb != nullptr) {
            m_cb();
        }
    }

   private:
    std::function<void()> m_cb;
};

}  // namespace talon

#endif  // TALON_RPC_RPC_CLOSURE_H
