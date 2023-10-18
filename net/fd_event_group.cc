//
// Created by cdy on 23-10-2.
//

#include "fd_event_group.h"

namespace talon {

    static FdEventGroup* g_fd_event_group = nullptr;

    FdEventGroup* FdEventGroup::GetFdEventGroup() {
        if (g_fd_event_group != nullptr) {
            return g_fd_event_group;
        }

        g_fd_event_group = new FdEventGroup(128);
        return g_fd_event_group;
    }

    FdEventGroup::FdEventGroup(int size) :m_size(size) {
        for (int i = 0; i < m_size; i++) {
            m_fd_group.push_back(new Fd_Event(i));
        }
    }

    FdEventGroup::~FdEventGroup() {
        for (int i = 0; i < m_size; ++i) {
            if (m_fd_group[i] != nullptr) {
                delete m_fd_group[i];
                m_fd_group[i] = nullptr;
            }
        }
    }

    Fd_Event* FdEventGroup::getFdEvent(int fd) {
        ScopeMutex<Mutex> lock(m_mutex);
        if ((size_t) fd < m_fd_group.size()) {
            return m_fd_group[fd];
        }

        int new_size = int(fd * 1.5);
        for (int i = m_fd_group.size(); i < new_size; ++i) {
            m_fd_group.push_back(new Fd_Event(i));
        }
        return m_fd_group[fd];

    }

} // talon