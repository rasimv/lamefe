#pragma once

#include "endian.h"
#include <filesystem>
#include "filereader.h"

namespace lamefe
{
    class WaveReader
    {
    public:
        class Exception: public lamefe::Exception
        {
        public:
            Exception(const std::string &a_what): lamefe::Exception(a_what) {}
        };

        enum Compression { WRCInt = 1, WRCFP = 3 };

        struct Chunk
        {
            uint32_t id;
            uint32_t size;
            Chunk(uint32_t a_id = 0, uint32_t a_size = 0): id(a_id), size(a_size) {}
        };

        WaveReader(FileReader &a_file);
        WaveReader(const WaveReader &) = delete;
        WaveReader(WaveReader &&) = delete;

        WaveReader &operator =(const WaveReader &) = delete;
        WaveReader &operator =(WaveReader &&) = delete;

        uint16_t compression() const { return m_compression; }
        uint16_t channels() const { return m_channels; }
        uint32_t sampleRate() const { return m_sampleRate; }
        uint16_t blockAlign() const { return m_blockAlign; }
        uint16_t bitsPerSample() const { return m_bitsPerSample; }
        uint32_t dataSize() const { return m_dataSize; }

        // Read sample data
        FileReader::Data data(uint64_t a_offset, size_t a_size);

    private:
        Chunk readChunk(uint64_t a_offset);
        Chunk findChunk(uint32_t a_id, uint64_t &a_beginOffset, uint64_t a_endOffset);
        void readFormat(uint64_t a_offset);

        FileReader &m_file;

        uint16_t m_compression = 0;
        uint16_t m_channels = 0;
        uint32_t m_sampleRate = 0;
        uint16_t m_blockAlign = 0;
        uint16_t m_bitsPerSample = 0;
        uint32_t m_dataSize = 0;

        uint64_t m_dataOffset = 0;
    };

    inline WaveReader::WaveReader(FileReader &a_file): m_file(a_file)
    {
        const auto l_riff(readChunk(0));
        if (l_riff.id != 0x46464952) throw Exception("Invalid wave");   // RIFF

        const auto l_wave(m_file.read(8, 4));
        if (l_wave.size() < 4 || lamefe::uint32le(l_wave.begin) != 0x45564157)    // WAVE
            throw Exception("Invalid wave");

        uint64_t l_offset = 8 + 4;
        const auto l_fmt(findChunk(0x20746d66, l_offset, uint64_t(8) + l_riff.size)); // fmt
        if (l_fmt.id != 0x20746d66) throw Exception("Invalid wave");
        readFormat(l_offset + 8);

        l_offset = 8 + 4;
        const auto l_data(findChunk(0x61746164, l_offset, uint64_t(8) + l_riff.size)); // data
        if (l_data.id != 0x61746164) throw Exception("Invalid wave");
        m_dataSize = l_data.size;
        m_dataOffset = l_offset + 8;
    }

    inline FileReader::Data WaveReader::data(uint64_t a_offset, size_t a_size)
    {
        const size_t l_toRead = a_offset + a_size <= m_dataSize? a_size:
                                a_offset < m_dataSize? m_dataSize - a_offset: 0;
        auto l_data(m_file.read(m_dataOffset + a_offset, l_toRead));
        if (l_data.size() > l_toRead) l_data.end = l_data.begin + l_toRead;
        return l_data;
    }

    inline WaveReader::Chunk WaveReader::readChunk(uint64_t a_offset)
    {
        const auto l_data(m_file.read(a_offset, 8));
        if (l_data.size() < 8) throw Exception("Invalid wave");
        return Chunk(lamefe::uint32le(l_data.begin), lamefe::uint32le(l_data.begin + 4));
    }

    inline WaveReader::Chunk WaveReader::findChunk(uint32_t a_id,
                                            uint64_t &a_beginOffset, uint64_t a_endOffset)
    {
        auto o = a_beginOffset;
        for (; o < a_endOffset; )
        {
            const auto l_one(readChunk(o));
            if (l_one.id == a_id) { a_beginOffset = o; return l_one; }
            o += 8 + (~uint64_t(1) & (uint64_t(1) + l_one.size));
        }
        a_beginOffset = o; return Chunk();
    }

    inline void WaveReader::readFormat(uint64_t a_offset)
    {
        const auto l_data(m_file.read(a_offset, 16));
        if (l_data.size() < 16) throw Exception("Invalid wave");

        m_compression = lamefe::uint16le(l_data.begin);
        m_channels = lamefe::uint16le(l_data.begin + 2);
        m_sampleRate = lamefe::uint32le(l_data.begin + 4);
        m_blockAlign = lamefe::uint16le(l_data.begin + 12);
        m_bitsPerSample = lamefe::uint16le(l_data.begin + 14);
    }
}
