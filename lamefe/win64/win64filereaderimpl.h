#pragma once

#include "win64filereader.h"
#include <filesystem>
#include "win64helper.h"
#include <memory>

namespace lamefe
{
    class Win64FileReaderImpl
    {
    public:
        class Exception: public Win64FileReader::Exception
        {
        public:
            Exception(const std::string &a_what): Win64FileReader::Exception(a_what) {}
        };

        Win64FileReaderImpl(const std::filesystem::path &a_path);
        Win64FileReaderImpl(const Win64FileReaderImpl &) = delete;
        Win64FileReaderImpl(Win64FileReaderImpl &&) = delete;

        Win64FileReaderImpl &operator =(const Win64FileReaderImpl &) = delete;
        Win64FileReaderImpl &operator =(Win64FileReaderImpl &&) = delete;

        ~Win64FileReaderImpl();

        uint64_t fileSize() { return m_fileSize; }
        FileReader::Data read(uint64_t a_offset, size_t a_size);
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
        uint64_t m_offset = 0, m_fileSize = 0;
        HandleUp m_mapping;
        ViewUp m_begin;
        uint8_t *m_end = nullptr;
    };

    inline Win64FileReaderImpl::Win64FileReaderImpl(const std::filesystem::path &a_path):
        m_gran(granularity()),
        m_file(::CreateFile(a_path.native().data(), GENERIC_READ,
                    0, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0), &closeHandle),
        m_mapping(INVALID_HANDLE_VALUE, &closeHandle), m_begin(nullptr, &unmapView)
    {
        if (m_file.get() == INVALID_HANDLE_VALUE) throw Exception("Create file error");

        DWORD l_high = 0;
        const DWORD l_low = ::GetFileSize(m_file.get(), &l_high);
        if (::GetLastError() != ERROR_SUCCESS) throw Exception("Get file size error");
        m_fileSize = l_low + (decltype(m_fileSize)(l_high) << 32);
    }

    inline Win64FileReaderImpl::~Win64FileReaderImpl() {}

    inline FileReader::Data Win64FileReaderImpl::read(uint64_t a_offset, size_t a_size)
    {
        if (a_size == 0 || m_fileSize <= a_offset) return FileReader::Data(nullptr, nullptr);

        if (m_offset <= a_offset && a_offset + a_size <= m_offset + (m_end - m_begin.get()))
            return FileReader::Data(m_begin.get() + (a_offset - m_offset), m_end);

        HandleUp l_mapping(::CreateFileMapping(m_file.get(),
                                        nullptr, PAGE_READONLY, 0, 0, nullptr), &closeHandle);
        if (l_mapping.get() == INVALID_HANDLE_VALUE)
            throw Exception("Create file mapping error");

        const uint64_t l_granBeginOffset = m_gran * (a_offset / m_gran);
        const uint64_t l_endOffset = a_offset + a_size,
            l_granEndOffset = m_gran * (l_endOffset / m_gran + (l_endOffset % m_gran != 0));
        const size_t l_granSize =
            (m_fileSize <= l_granEndOffset? m_fileSize: l_granEndOffset) - l_granBeginOffset;

        ViewUp l_begin(static_cast<decltype(m_begin.get())>
                (::MapViewOfFile(l_mapping.get(), FILE_MAP_READ,
                l_granBeginOffset >> 32, DWORD(l_granBeginOffset), l_granSize)), &unmapView);
        if (l_begin == nullptr) throw Exception("Map view of file error");

        m_offset = l_granBeginOffset;
        std::swap(l_mapping, m_mapping);
        std::swap(l_begin, m_begin);
        m_end = m_begin.get() + l_granSize;

        return FileReader::Data(m_begin.get() + (a_offset - m_offset), m_end);
    }

    inline void Win64FileReaderImpl::clear()
    {
        m_begin.reset();
        m_end = nullptr;
        m_mapping.reset();
        m_offset = 0;
    }
}
