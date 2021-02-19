#pragma once

#include "win64filewriter.h"
#include "win64filewriterimpl.h"

namespace lamefe
{
    Win64FileWriter::Win64FileWriter(const std::filesystem::path &a_path):
        m_impl(new Win64FileWriterImpl(a_path)) {}

    Win64FileWriter::~Win64FileWriter() { delete m_impl; }

    FileWriter::Data Win64FileWriter::write(uint64_t a_offset, size_t a_size)
    {
        return m_impl->write(a_offset, a_size);
    }

    void Win64FileWriter::flush(size_t a_size, bool a_flushUnderlying)
    {
        m_impl->flush(a_size, a_flushUnderlying);
    }

    void Win64FileWriter::clear() { m_impl->clear(); }
}
