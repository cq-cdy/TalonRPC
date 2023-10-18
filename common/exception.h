//
// Created by cdy on 23-10-18.
//

#ifndef TALON_RPC_EXCEPTION_H
#define TALON_RPC_EXCEPTION_H


#include <exception>
#include <string>
#include <utility>
namespace talon{
    class TalonException : public std::exception {
    public:

        TalonException(int error_code, std::string  error_info) : m_error_code(error_code), m_error_info(std::move(error_info)) {}

        // 异常处理
        // 当捕获到 talonException 及其子类对象的异常时，会执行该函数
        virtual void handle() = 0;

        virtual ~TalonException() {};

        int errorCode() const {
            return m_error_code;
        }

        std::string errorInfo() {
            return m_error_info;
        }

    protected:
        int m_error_code {0};

        std::string m_error_info;

    };

}
#endif //TALON_RPC_EXCEPTION_H