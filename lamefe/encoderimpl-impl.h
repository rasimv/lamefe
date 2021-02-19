#pragma once

#include "encoderimpl.h"

namespace lamefe
{
    inline EncoderImpl::EncoderImpl(OStream &a_log): m_log(a_log) { createThreads(); }

    inline EncoderImpl::~EncoderImpl() { shutdownThreads(); }

    inline void EncoderImpl::run()
    {
        // Initiate processing files (in worker threads)
        std::unique_lock<decltype(m_inputMutex)> l_lock(m_inputMutex);
        m_inputCount = 0; m_inputSize = m_files.size(); m_processed = 0;
        l_lock.unlock();
        m_inputCv.notify_all();

        // Process all files
        while (m_processed < m_inputSize) runJob();
        if (m_processed > 0) m_log << m_processed << " wave-file(s) processed" << std::endl;
    }

    inline void EncoderImpl::runJob()
    {
        std::unique_lock<decltype(m_mainMutex)> l_lock(m_mainMutex);

        // Wait until
        // - shutdown requested from a worker thread (due a fatal error) OR
        // - all files processed OR
        // - new message(s) to print arrived OR
        // - timeout (1000 ms) -- handle theoretical case when reqShutdown() throws an exception
        // Most of time the main thread idles here
        m_mainCv.wait_for(l_lock, std::chrono::milliseconds(1000),
            [&] { return m_reqShutdown || m_processed >= m_inputSize || !m_messages.empty(); });

        // Leave if shutdown requested
        if (m_reqShutdown) throw Exception("Exception in thread");

        // Print new message(s)
        const auto l_messages(std::move(m_messages));
        l_lock.unlock();
        for (const auto &q: l_messages) m_log << q << std::endl;
    }

    inline void EncoderImpl::createThreads() try
    {
        for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
            m_array.emplace_back(&EncoderImpl::worker, this);
    }
    catch (...) { shutdownThreads(); throw; }

    inline void EncoderImpl::shutdownThreads()
    {
        std::unique_lock<decltype(m_inputMutex)> l_lock(m_inputMutex);
        m_shutdown = true;
        l_lock.unlock();
        m_inputCv.notify_all();
        for (auto &l_thread: m_array) l_thread.join();
    }
}
