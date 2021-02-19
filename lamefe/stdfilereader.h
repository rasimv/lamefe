#pragma once

#include "filereader.h"
#include <filesystem>
#include <fstream>

namespace lamefe
{
    class StdFileReader: public FileReader
    {
    public:
        class Exception: public FileReader::Exception
        {
        public:
            Exception(const std::string &a_what): FileReader::Exception(a_what) {}
        };

        constexpr size_t minAlloc() { return 4096; }

        StdFileReader(const std::filesystem::path &a_path);
        StdFileReader(const StdFileReader &) = delete;
        StdFileReader(StdFileReader &&a) noexcept { swap(a, *this); }

        StdFileReader &operator =(const StdFileReader &) = delete;
        StdFileReader &operator =(StdFileReader &&a) noexcept { swap(a, *this); return *this; }

        ~StdFileReader() override { delete []m_begin; }

        uint64_t fileSize() override { return m_fileSize; }
        Data read(uint64_t a_offset, size_t a_size) override;
        void clear() override;

        friend void swap(StdFileReader &o, StdFileReader &a) noexcept;

    private:
        uint8_t *m_begin = nullptr, *m_actual = nullptr, *m_end = nullptr;
        uint64_t m_offset = 0;
        std::ifstream m_ifs;
        uint64_t m_fileSize = 0;
    };

    inline StdFileReader::StdFileReader(const std::filesystem::path &a_path):
        m_ifs(a_path, std::ios::binary | std::ios::in),
        m_fileSize(std::filesystem::file_size(a_path))
    {
        if (!m_ifs.is_open() || m_ifs.bad()) throw Exception("File open error");
    }

    inline FileReader::Data StdFileReader::read(uint64_t a_offset, size_t a_size)
    {
        if (m_offset <= a_offset && a_offset + a_size <= m_offset + (m_actual - m_begin))
            return Data(m_begin + (a_offset - m_offset), m_actual);

        if (size_t(m_end - m_begin) < a_size)
        {
            const auto l_newSize = std::max(a_size, minAlloc());
            const auto l_delete = m_begin;
            m_begin = m_actual = new uint8_t[l_newSize];
            delete []l_delete;
            m_end = m_begin + l_newSize;
        }

        m_ifs.clear();
        if (a_offset != m_ifs.tellg()) m_ifs.seekg(a_offset);
        if (m_ifs.bad()) throw Exception("File seek error");
        m_offset = a_offset;
        m_ifs.read(reinterpret_cast<char *>(m_begin), m_end - m_begin);
        if (m_ifs.bad()) throw Exception("File read error");
        m_actual = m_begin + m_ifs.gcount();

        return Data(m_begin, m_actual);
    }

    inline void StdFileReader::clear()
    {
        delete []m_begin;
        m_begin = m_actual = m_end = nullptr;
    }

    inline void swap(StdFileReader &o, StdFileReader &a) noexcept
    {
        std::swap(o.m_begin, a.m_begin);
        std::swap(o.m_actual, a.m_actual);
        std::swap(o.m_end, a.m_end);
        std::swap(o.m_offset, a.m_offset);
        std::swap(o.m_ifs, a.m_ifs);
    }
}
