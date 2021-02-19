#pragma once

#include "encoderimpl.h"
#include "wavereader.h"
#include "lamecontext.h"

namespace lamefe
{
    // Return sample type and sample size supported in lame
    // according to wave compression (only raw is supported) and sample size
    inline std::pair<EncoderImpl::SampleType, unsigned>
                                sampleType(uint16_t a_compression, unsigned a_1cSampleSize)
    {
        if (a_compression == WaveReader::WRCInt)
        {
            if (a_1cSampleSize == 1 || a_1cSampleSize == 2) return {EncoderImpl::STS16, 2};
            if (a_1cSampleSize == 3 || a_1cSampleSize == 4) return {EncoderImpl::STS32, 4};
            throw Exception("Unsupported bit format wave");
        }
        if (a_compression == WaveReader::WRCFP)
        {
            if (a_1cSampleSize == 4) return {EncoderImpl::STF32, 4};
            if (a_1cSampleSize == 8) return {EncoderImpl::STF64, 8};
            throw Exception("Unsupported bit format wave");
        }
        throw Exception("Unsupported compression wave");
    }

    // Resize sample chunk from one sample size to another
    inline void resize(unsigned a_channelNum,
                            unsigned a_1cSampleSize, const void *a_input, unsigned a_sampleNum,
                            unsigned a_resize1cSampleSize, void *a_output)
    {
        auto l_input = static_cast<const uint8_t *>(a_input);
        auto l_output = static_cast<uint8_t *>(a_output);
        for (unsigned i = 0; i < a_sampleNum; i++)
            for (unsigned j = 0; j < a_channelNum;
                j++, l_input += a_1cSampleSize, l_output += a_resize1cSampleSize)
            {
                if (a_1cSampleSize == 1)
                    uint16(l_output) = (int16_t(*l_input) - 128) << 8;
                else if (a_1cSampleSize == 3)
                    uint32(l_output) = uint32_t(*l_input) << 8 |
                                        uint32_t(l_input[1]) << 16 | uint32_t(l_input[2]) << 24;
        }
    }

    // Toggle endianness of sample chunk
    inline void toggleEndian(unsigned a_channelNum,
                                    unsigned a_sampleSize, void *a_input, unsigned a_sampleNum)
    {
        auto l_input = static_cast<uint8_t *>(a_input);
        for (unsigned i = 0; i < a_channelNum * a_sampleNum; i++, l_input += a_sampleSize)
        {
            switch (a_sampleSize)
            {
            case 2: uint16(l_input) = uint16le(l_input); break;
            case 4: uint32(l_input) = uint32le(l_input); break;
            case 8: uint64(l_input) = uint64le(l_input); break;
            }
        }
    }

    // Call lame encode version according to channel number and sample type
    inline int lameEncode(unsigned a_channelNum, EncoderImpl::SampleType a_sampleType,
                lame_t gfp, void *pcm, int nsamples, unsigned char *mp3buf, int mp3buf_size)
    {
        if (nsamples <= 0) return ::lame_encode_flush(gfp, mp3buf, mp3buf_size);
        switch (a_sampleType)
        {
        case EncoderImpl::STS16:
            return a_channelNum == 1? ::lame_encode_buffer(gfp,
                            static_cast<short *>(pcm), nullptr, nsamples, mp3buf, mp3buf_size):
                                        ::lame_encode_buffer_interleaved(gfp,
                            static_cast<short *>(pcm), nsamples, mp3buf, mp3buf_size);
        case EncoderImpl::STS32:
            return a_channelNum == 1? ::lame_encode_buffer_int(gfp,
                            static_cast<int *>(pcm), nullptr, nsamples, mp3buf, mp3buf_size):
                                        ::lame_encode_buffer_interleaved_int(gfp,
                            static_cast<int *>(pcm), nsamples, mp3buf, mp3buf_size);
        case EncoderImpl::STF32:
            return a_channelNum == 1? ::lame_encode_buffer_ieee_float(gfp,
                            static_cast<float *>(pcm), nullptr, nsamples, mp3buf, mp3buf_size):
                                        ::lame_encode_buffer_interleaved_ieee_float(gfp,
                            static_cast<float *>(pcm), nsamples, mp3buf, mp3buf_size);
        case EncoderImpl::STF64:
            return a_channelNum == 1? ::lame_encode_buffer_ieee_double(gfp,
                            static_cast<double *>(pcm), nullptr, nsamples, mp3buf, mp3buf_size):
                                        ::lame_encode_buffer_interleaved_ieee_double(gfp,
                            static_cast<double *>(pcm), nsamples, mp3buf, mp3buf_size);
        }
        return -1;
    }

    inline String formatString(const WaveReader &a_wr)
    {
        SStream l_ss;
        l_ss << a_wr.sampleRate() << " | ";
        if (a_wr.channels() > 0) l_ss << a_wr.channels() << "x";
        l_ss << a_wr.bitsPerSample();
        l_ss << (a_wr.compression() == WaveReader::WRCInt? "i":
                    a_wr.compression() == WaveReader::WRCFP? "f": "x");
        l_ss << " | " << a_wr.dataSize();
        return l_ss.str();
    }

    // Set encoding parameters in lame context
    inline void applySettings(const lamefe::LameContext &a_context,
                                                            const lamefe::Settings &a_output)
    {
        if (a_output.quality)
            if (::lame_set_quality(a_context.gf, a_output.quality.value()) != 0)
                throw Exception("Lame set quality error");

        if (!a_output.mode) return;

        switch (a_output.mode.value())
        {
        case Settings::SMCBR:
            if (::lame_set_VBR(a_context.gf, vbr_off) != 0)
                throw Exception("Lame set vbr error");
            if (!a_output.value) break;
            if (::lame_set_brate(a_context.gf, a_output.value.value()) != 0)
                throw Exception("Lame set brate error");
            break;
        case Settings::SMABR:
            if (::lame_set_VBR(a_context.gf, vbr_abr) != 0)
                throw Exception("Lame set vbr error");
            if (!a_output.value) break;
            if (::lame_set_VBR_mean_bitrate_kbps(a_context.gf, a_output.value.value()) != 0)
                throw Exception("Lame set vbr mean bitrate error");
            break;
        case Settings::SMVBR:
            if (::lame_set_VBR(a_context.gf, vbr_mtrh) != 0)
                throw Exception("Lame set vbr error");
            if (!a_output.value) break;
            if (::lame_set_VBR_q(a_context.gf, a_output.value.value()) != 0)
                throw Exception("Lame set vbr quality error");
            break;
        }
    }
}
