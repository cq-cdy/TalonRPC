//
// Created by cdy on 23-9-30.
//

#include "iothreadgroup.h"


talon::IOThreadGroup::IOThreadGroup(int size) : m_size(size) {
    m_io_thread_groups.resize(m_size);
    for (size_t i = 0; i < m_size; i++) {
        m_io_thread_groups[i] = new IOThread();
    } //  start ,but wait in sem_t
}

talon::IOThreadGroup::~IOThreadGroup() {

}

void talon::IOThreadGroup::start() {
    for (auto &m_io_thread_group: m_io_thread_groups) {
        m_io_thread_group->start();
    }
}

void talon::IOThreadGroup::join() {
    int c = 0;
    for (auto &m_io_thread_group: m_io_thread_groups) {
        DEBUGLOG("IOThread join---------------------------------------");

        DEBUGLOG("IOThread %d join success", c++);

        m_io_thread_group->join();
    }
}

talon::IOThread *talon::IOThreadGroup::getIOThread() {
    if (m_index == (int) m_io_thread_groups.size() || m_index == -1) {
        m_index = 0;
    }
    return m_io_thread_groups[m_index++];
}

std::vector<talon::IOThread *>& talon::IOThreadGroup::getIOThread_group() {
    return m_io_thread_groups;
}




