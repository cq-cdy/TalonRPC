//
// Created by cdy on 23-10-1.
//

#ifndef TALON_RPC_TCP_BUFFER_H
#define TALON_RPC_TCP_BUFFER_H
#include "vector"
#include "memory"
namespace talon {

    class TcpBuffer {
    public:

        typedef std::shared_ptr<TcpBuffer> s_ptr;

        TcpBuffer(int size);

        ~TcpBuffer();

        // 返回可读字节数
        int readAble() const ;

        // 返回可写的字节数
        int writeAble() const ;

        int readIndex() const ;

        int writeIndex() const ;

        void writeToBuffer(const char* buf, int size);

        void readFromBuffer(std::vector<char>& re, int size);

        void resizeBuffer(int new_size);

        void adjustBuffer();

        void moveReadIndex(int size);

        void moveWriteIndex(int size);

    private:
        int m_read_index {0};
        int m_write_index {0};
        int m_size {0};

    public:
        std::vector<char> m_buffer;
    };

} // talon

#endif //TALON_RPC_TCP_BUFFER_H
