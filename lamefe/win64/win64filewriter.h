#pragma once

#include "../filewriter.h"
#include <filesystem>

namespace lamefe
{
    class Win64FileWriterImpl;

    class Win64FileWriter: public FileWriter
    {
    public:
        class Exception: public FileWriter::Exception
        {
        public:
            Exception(const std::string &a_what): FileWriter::Exception(a_what) {}
        };

        Win64FileWriter(const std::filesystem::path &a_path);
        Win64FileWriter(const Win64FileWriter &) = delete;
        Win64FileWriter(Win64FileWriter &&a) noexcept { swap(a, *this); }

        Win64FileWriter &operator =(const Win64FileWriter &) = delete;
        Win64FileWriter &operator =(Win64FileWriter &&a) noexcept
        {
            swap(a, *this);
            return *this;
        }

        ~Win64FileWriter() override;

        Data write(uint64_t a_offset, size_t a_size) override;
        void flush(size_t a_size, bool a_flushUnderlying = false) override;
        void clear() override;

        friend void swap(Win64FileWriter &o, Win64FileWriter &a) noexcept
        {
            std::swap(o.m_impl, a.m_impl);
        }

    private:
        Win64FileWriterImpl *m_impl = nullptr;
    };
}
