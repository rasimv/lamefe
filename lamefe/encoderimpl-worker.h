#pragma once

#include "encoderimpl.h"

namespace lamefe
{
    inline void EncoderImpl::reqShutdown() try
    {
        std::unique_lock<decltype(m_mainMutex)> l_lock(m_mainMutex);
        m_reqShutdown = true;
        l_lock.unlock();
        m_mainCv.notify_all();
    }
    catch (...) { m_reqShutdown = true; }

    inline void EncoderImpl::worker() try { while (!m_shutdown) workerSub(); }
    catch (const std::exception &a)
    {
        reqShutdown();
        std::cerr << std::endl << "Exception: " << a.what() << std::endl;
    }
    catch (...)
    {
        reqShutdown();
        std::cerr << std::endl << "Exception: ..." << std::endl;
    }

    inline void EncoderImpl::workerSub()
    {
        // Wait until
        // - shutdown state (initiated in the main thread)
        // - there is a file in queue to process
        std::unique_lock<decltype(m_inputMutex)> l_inputLock(m_inputMutex);
        m_inputCv.wait(l_inputLock, [&] { return m_shutdown || m_inputCount < m_inputSize; });
        if (m_shutdown) return;
        const auto l_index = m_inputCount++;
        l_inputLock.unlock();

        // Process the file
        process(l_index);

        // Increment processed counter and notify the main thread
        std::unique_lock<decltype(m_mainMutex)> l_mainLock(m_mainMutex);
        ++m_processed;
        l_mainLock.unlock();
        m_mainCv.notify_all();
    }

    inline void EncoderImpl::process(size_t a_index)
    {
        const auto &l_filename = m_files[a_index];
        try { processSub(l_filename); }
        catch (const std::exception &a) { fileLog(l_filename, String(), a.what()); }
    }
}
