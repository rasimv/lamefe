#include "encoderimpl-helper.h"
#include "stdfilereader.h"
#include "stdfilewriter.h"

namespace lamefe
{
    void EncoderImpl::processSub(const std::filesystem::path &a_filename)
    {
        // Make file paths
        using Path = decltype(m_dir);
        const auto l_inputFilepath(Path(m_dir) /= a_filename);
        const auto l_outputFilename(Path(a_filename.stem()) += ".mp3");
        const auto l_outputFilepath(Path(m_dir) /= l_outputFilename);

        // Open input file and read wave
        StdFileReader l_input(l_inputFilepath);
        WaveReader l_wave(l_input);

        fileLog(a_filename, formatString(l_wave));

        // Create output file
        StdFileWriter l_output(l_outputFilepath);

        // Create lame context
        const LameContext l_context;
        if (l_context.gf == nullptr) throw Exception("Lame init error");

        const unsigned l_1cSampleSize = (l_wave.bitsPerSample() + 7) / 8;
        const unsigned l_blockSize = l_1cSampleSize * l_wave.channels();
        const decltype(l_wave.dataSize()) l_sampleNum = l_wave.dataSize() / l_blockSize;

        // Set input parameters
        if (::lame_set_num_samples(l_context.gf, l_sampleNum) != 0)
            throw Exception("Lame set num samples error");
        if (::lame_set_in_samplerate(l_context.gf, l_wave.sampleRate()) != 0)
            throw Exception("Lame set in-samplerate error");
        if (::lame_set_num_channels(l_context.gf, l_wave.channels()) != 0)
            throw Exception("Lame set num channels error");

        // Set encoding/output parameters
        applySettings(l_context, m_settings);

        // Init params
        if (::lame_init_params(l_context.gf) != 0)
            throw Exception("Lame init params error");

        // Allocate a buffer if wave data resizing is needed
        // (when encoding 8- and 24-bit waves)
        const unsigned l_chunkSampleNum = 1152;
        std::unique_ptr<uint8_t []> l_resized;
        const auto l_sampleType(sampleType(l_wave.compression(), l_1cSampleSize));
        const unsigned l_resizeBlockSize = l_sampleType.second * l_wave.channels();
        if (l_1cSampleSize != l_sampleType.second)
                    l_resized.reset(new uint8_t[size_t(l_resizeBlockSize) * l_chunkSampleNum]);

        // Encoding loop (a chunk per iteration)
        const unsigned l_chunkSize = l_blockSize * l_chunkSampleNum;
        const unsigned l_maxEncSize = unsigned(1.25 * l_chunkSampleNum + 7200);
        uint64_t l_writeOffset = 0;
        for (decltype(l_wave.dataSize()) l_readOffset = 0; ; )
        {
            // Read the chunk and prepare write
            const auto l_data(l_wave.data(l_readOffset, l_chunkSize));
            const auto l_enc(l_output.write(l_writeOffset, l_maxEncSize));

            const unsigned l_readSampleNum = unsigned(l_data.size() / l_blockSize);

            // Resize the chunk if needed
            if (l_resized != nullptr)
                resize(l_wave.channels(), l_1cSampleSize, l_data.begin, l_readSampleNum,
                                            l_sampleType.second, l_resized.get());

            // The resized chunk is passed to lame if resizing is needed,
            // the original read buffer is passed otherwise
            const auto l_dataP = l_resized != nullptr? l_resized.get(): l_data.begin;

            // Toggle chunk data endianness if the target machine is big-endian
            // (wave is little-endian)
            if (bigEndian())
                toggleEndian(l_wave.channels(), l_sampleType.second, l_dataP, l_readSampleNum);

            // Encode the chunk
            const auto l_encSize = lameEncode(l_wave.channels(), l_sampleType.first,
                l_context.gf, l_dataP, int(l_readSampleNum), l_enc.begin, int(l_enc.size()));
            if (l_encSize < 0) throw Exception("Lame encode error");

            l_output.flush(l_encSize);

            // End the loop if all the wave data is processed
            if (l_readSampleNum <= 0) break;

            l_readOffset += l_blockSize * l_readSampleNum;
            l_writeOffset += l_encSize;
        }

        fileLog(l_outputFilename, LAMEFE_S("written"));
    }
}
