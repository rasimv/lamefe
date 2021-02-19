#pragma once

#include "../filereader.h"
#include <filesystem>

namespace lamefe
{
    class Win64FileReaderImpl;

    class Win64FileReader: public FileReader
    {
    public:
        class Exception: public FileReader::Exception
        {
        public:
            Exception(const std::string &a_what): FileReader::Exception(a_what) {}
        };

        Win64FileReader(const std::filesystem::path &a_path);
        Win64FileReader(const Win64FileReader &) = delete;
        Win64FileReader(Win64FileReader &&a) noexcept { swap(a, *this); }

        Win64FileReader &operator =(const Win64FileReader &) = delete;
        Win64FileReader &operator =(Win64FileReader &&a) noexcept
        {
            swap(a, *this);
            return *this;
        }

        ~Win64FileReader() override;

        uint64_t fileSize() override;
        Data read(uint64_t a_offset, size_t a_size) override;
        void clear() override;

        friend void swap(Win64FileReader &o, Win64FileReader &a) noexcept
        {
            std::swap(o.m_impl, a.m_impl);
        }

    private:
        Win64FileReaderImpl *m_impl = nullptr;
    };
}
