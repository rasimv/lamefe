#pragma once

#include "win64filewriter.h"
#include <filesystem>
#include "win64helper.h"
#include <memory>

namespace lamefe
{
    class Win64FileWriterImpl
    {
    public:
        class Exception: public Win64FileWriter::Exception
        {
        public:
            Exception(const std::string &a_what): Win64FileWriter::Exception(a_what) {}
        };

        Win64FileWriterImpl(const std::filesystem::path &a_path);
        Win64FileWriterImpl(const Win64FileWriterImpl &) = delete;
        Win64FileWriterImpl(Win64FileWriterImpl &&) = delete;

        Win64FileWriterImpl &operator =(const Win64FileWriterImpl &) = delete;
        Win64FileWriterImpl &operator =(Win64FileWriterImpl &&) = delete;

        ~Win64FileWriterImpl();

        FileWriter::Data write(uint64_t a_offset, size_t a_size);
        void flush(size_t a_size, bool a_flushUnderlying = false);
        void clear();

    private:
        static BOOL closeHandle(void *a)
        {
            return a == INVALID_HANDLE_VALUE? 0: ::CloseHandle(a);
        }

        static BOOL unmapView(void *a)
        {
            return a == nullptr? 0: ::UnmapViewOfFile(a);
        }

        using HandleUp = std::unique_ptr<void, decltype(&closeHandle)>;
        using ViewUp = std::unique_ptr<uint8_t [], decltype(&unmapView)>;

        size_t m_gran = 65536;
        HandleUp m_file;
        uint64_t m_offset = 0, m_reqOffset = 0, m_fileSize = 0;
        HandleUp m_mapping;
        ViewUp m_begin;
        uint8_t *m_end = nullptr;
    };

    inline Win64FileWriterImpl::Win64FileWriterImpl(const std::filesystem::path &a_path):
        m_gran(granularity()),
        m_file(::CreateFile(a_path.native().data(), GENERIC_READ | GENERIC_WRITE,
                    0, nullptr, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0), &closeHandle),
        m_mapping(INVALID_HANDLE_VALUE, &closeHandle), m_begin(nullptr, &unmapView)
    {
        if (m_file.get() == INVALID_HANDLE_VALUE) throw Exception("Create file error");
    }

    inline Win64FileWriterImpl::~Win64FileWriterImpl() { try { clear(); } catch (...) {} }

    inline FileWriter::Data Win64FileWriterImpl::write(uint64_t a_offset, size_t a_size)
    {
        if (a_size == 0)
        {
            m_reqOffset = a_offset;
            return FileWriter::Data(nullptr, nullptr);
        }

        if (m_offset <= a_offset && a_offset + a_size <= m_offset + (m_end - m_begin.get()))
        {
            m_reqOffset = a_offset;
            return FileWriter::Data(m_begin.get() + (a_offset - m_offset), m_end);
        }

        const uint64_t l_endOffset = a_offset + a_size,
            l_granEndOffset = m_gran * (l_endOffset / m_gran + (l_endOffset % m_gran != 0));
        HandleUp l_mapping(::CreateFileMapping(m_file.get(), nullptr, PAGE_READWRITE,
                        l_granEndOffset >> 32, DWORD(l_granEndOffset), nullptr), &closeHandle);
        if (l_mapping.get() == INVALID_HANDLE_VALUE)
            throw Exception("Create file mapping error");

        const uint64_t l_granBeginOffset = m_gran * (a_offset / m_gran);
        const size_t l_granSize = l_granEndOffset - l_granBeginOffset;

        ViewUp l_begin(static_cast<decltype(m_begin.get())>
                (::MapViewOfFile(l_mapping.get(), FILE_MAP_WRITE,
                l_granBeginOffset >> 32, DWORD(l_granBeginOffset), l_granSize)), &unmapView);
        if (l_begin == nullptr) throw Exception("Map view of file error");

        m_offset = l_granBeginOffset;
        m_reqOffset = a_offset;
        std::swap(l_mapping, m_mapping);
        std::swap(l_begin, m_begin);
        m_end = m_begin.get() + l_granSize;

        return FileWriter::Data(m_begin.get() + (a_offset - m_offset), m_end);
    }

    inline void Win64FileWriterImpl::flush(size_t a_size, bool a_flushUnderlying)
    {
        const decltype(m_fileSize) l_fileSize = m_reqOffset +
            (a_size < size_t(m_end - m_begin.get())? a_size: size_t(m_end - m_begin.get()));
        if (l_fileSize > m_fileSize) m_fileSize = l_fileSize;
    }

    inline void Win64FileWriterImpl::clear()
    {
        m_begin.reset();
        m_end = nullptr;
        m_mapping.reset();
        m_offset = 0;
        m_reqOffset = 0;

        LONG l_high = m_fileSize >> 32;
        ::SetFilePointer(m_file.get(), LONG(m_fileSize), &l_high, FILE_BEGIN);
        if (::GetLastError() != ERROR_SUCCESS) throw Exception("Set file pointer error");
        if (!::SetEndOfFile(m_file.get())) throw Exception("Set end of file error");
    }
}
