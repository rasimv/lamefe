#pragma once

#include "encoderimpl.h"

namespace lamefe
{
    inline void EncoderImpl::log(String &&a_message)
    {
        std::unique_lock<decltype(m_mainMutex)> l_lock(m_mainMutex);
        m_messages.push_back(std::move(a_message));
        l_lock.unlock();
        m_mainCv.notify_all();
    }

    inline void EncoderImpl::fileLog(const std::filesystem::path &a_filename,
                                                const String &a_message, const char *a_what)
    {
        SStream l_ss;
        l_ss << a_filename;
        if (!a_message.empty()) l_ss << " - " << a_message;
        if (a_what != nullptr) l_ss << " - " << a_what;
        log(l_ss.str());
    }
}
