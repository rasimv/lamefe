#pragma once

#include "filewriter.h"
#include <filesystem>
#include <fstream>

namespace lamefe
{
    class StdFileWriter: public FileWriter
    {
    public:
        class Exception: public FileWriter::Exception
        {
        public:
            Exception(const std::string &a_what): FileWriter::Exception(a_what) {}
        };

        constexpr size_t minAlloc() { return 4096; }

        StdFileWriter(const std::filesystem::path &a_path);
        StdFileWriter(const StdFileWriter &) = delete;
        StdFileWriter(StdFileWriter &&a) noexcept { swap(a, *this); }

        StdFileWriter &operator =(const StdFileWriter &) = delete;
        StdFileWriter &operator =(StdFileWriter &&a) noexcept { swap(a, *this); return *this; }

        ~StdFileWriter() override { delete []m_begin; }

        Data write(uint64_t a_offset, size_t a_size) override;
        void flush(size_t a_size, bool a_flushUnderlying = false) override;
        void clear() override;

        friend void swap(StdFileWriter &o, StdFileWriter &a) noexcept;

    private:
        uint8_t *m_begin = nullptr, *m_end = nullptr;
        uint64_t m_offset = 0;
        std::ofstream m_ofs;
    };

    inline StdFileWriter::StdFileWriter(const std::filesystem::path &a_path):
        m_ofs(a_path, std::ios::binary | std::ios::out)
    {
        if (!m_ofs.is_open() || m_ofs.bad()) throw Exception("File open error");
    }

    inline FileWriter::Data StdFileWriter::write(uint64_t a_offset, size_t a_size)
    {
        if (size_t(m_end - m_begin) < a_size)
        {
            const auto l_newSize = std::max(a_size, minAlloc());
            const auto l_delete = m_begin;
            m_begin = new uint8_t[l_newSize];
            delete []l_delete;
            m_end = m_begin + l_newSize;
        }
        m_offset = a_offset;
        return Data(m_begin, m_end);
    }

    inline void StdFileWriter::flush(size_t a_size, bool a_flushUnderlying)
    {
        m_ofs.clear();
        if (m_offset != m_ofs.tellp()) m_ofs.seekp(m_offset);
        if (m_ofs.bad()) throw Exception("File seek error");
        m_ofs.write(reinterpret_cast<const char *>(m_begin),
                    std::min(a_size, size_t(m_end - m_begin)));
        if (m_ofs.bad()) throw Exception("File write error");
        if (a_flushUnderlying) m_ofs.flush();
    }

    inline void StdFileWriter::clear()
    {
        delete []m_begin;
        m_begin = m_end = nullptr;
    }

    inline void swap(StdFileWriter &o, StdFileWriter &a) noexcept
    {
        std::swap(o.m_begin, a.m_begin);
        std::swap(o.m_end, a.m_end);
        std::swap(o.m_offset, a.m_offset);
        std::swap(o.m_ofs, a.m_ofs);
    }
}
