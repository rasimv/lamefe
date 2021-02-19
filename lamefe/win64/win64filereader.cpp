#pragma once

#include "win64filereader.h"
#include "win64filereaderimpl.h"

namespace lamefe
{
    Win64FileReader::Win64FileReader(const std::filesystem::path &a_path):
        m_impl(new Win64FileReaderImpl(a_path)) {}

    Win64FileReader::~Win64FileReader() { delete m_impl; }

    uint64_t Win64FileReader::fileSize() { return m_impl->fileSize(); }

    FileReader::Data Win64FileReader::read(uint64_t a_offset, size_t a_size)
    {
        return m_impl->read(a_offset, a_size);
    }

    void Win64FileReader::clear() { m_impl->clear(); }
}
