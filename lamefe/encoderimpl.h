#pragma once

#include "encoder.h"
#include <vector>
#include <atomic>
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace lamefe
{
    class EncoderImpl
    {
    public:
        class Exception: public Encoder::Exception
        {
        public:
            Exception(const std::string &a_what): Encoder::Exception(a_what) {}
        };

        enum SampleType { STS16, STS32, STF32, STF64 };

        EncoderImpl(OStream &a_log);
        EncoderImpl(const EncoderImpl &) = delete;
        EncoderImpl(EncoderImpl &&) = delete;

        EncoderImpl &operator =(const EncoderImpl &) = delete;
        EncoderImpl &operator =(EncoderImpl &&) = delete;

        ~EncoderImpl();

        void set(Settings &&a);
        void list(std::filesystem::path &&a_dir);
        void run();

    private:
        void runJob();

        void createThreads();
        void shutdownThreads();

        // The rest methods are called in worker threads
        void log(String &&a_message);
        void fileLog(const std::filesystem::path &a_filename,
                                const String &a_message, const char *a_what = nullptr);

        void reqShutdown();
        void worker();                  // Worker threads execution function
        void workerSub();
        void process(size_t a_index);

        void processSub(const std::filesystem::path &a_filename);

        OStream &m_log;

        std::atomic_bool m_shutdown = false, m_reqShutdown = false;
        std::vector<std::thread> m_array;

        Settings m_settings;
        std::filesystem::path m_dir;
        std::vector<std::filesystem::path> m_files;

        size_t m_inputCount = 0;            // Index of a file to process next
        size_t m_inputSize = 0;             // Number of files to process
        std::atomic_size_t m_processed;
        std::mutex m_inputMutex;
        std::condition_variable m_inputCv;

        std::deque<String> m_messages;
        std::mutex m_mainMutex;
        std::condition_variable m_mainCv;
    };
}

#include "encoderimpl-impl.h"
#include "encoderimpl-set.h"
#include "encoderimpl-log.h"
#include "encoderimpl-worker.h"
