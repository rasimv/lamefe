#pragma once

#include <cstdint>

//#define LAMEFE_BIG_ENDIAN 1

namespace lamefe
{
    constexpr bool bigEndian()
    {
        return
#ifdef LAMEFE_BIG_ENDIAN
            true
#else
            false
#endif
            ;
    }

    inline uint16_t &uint16(void *a) { return *static_cast<uint16_t *>(a); }
    inline const uint16_t &uint16(const void *a) { return *static_cast<const uint16_t *>(a); }
    inline uint32_t &uint32(void *a) { return *static_cast<uint32_t *>(a); }
    inline const uint32_t &uint32(const void *a) { return *static_cast<const uint32_t *>(a); }
    inline uint64_t &uint64(void *a) { return *static_cast<uint64_t *>(a); }
    inline const uint64_t &uint64(const void *a) { return *static_cast<const uint64_t *>(a); }

    inline uint16_t uint16le(const void *a)
    {
        const uint16_t q = uint16(a);
#ifdef LAMEFE_BIG_ENDIAN
        return q >> 8 | q << 8;
#else
        return q;
#endif
    }

    inline uint32_t uint32le(const void *a)
    {
        const uint32_t q = uint32(a);
#ifdef LAMEFE_BIG_ENDIAN
        return q >> 3 * 8 | 0xff00 & q >> 8 | 0xff0000 & q << 8 | q << 3 * 8;
#else
        return q;
#endif
    }

    inline uint64_t uint64le(const void *a)
    {
        const uint64_t q = uint64(a);
#ifdef LAMEFE_BIG_ENDIAN
        return  q >> 7 * 8 | 0xff00 & q >> 5 * 8 |
            0xff0000 & q >> 3 * 8 | 0xff000000 & q >> 8 | 0xff00000000 & q << 8 |
            0xff0000000000 & q << 3 * 8 | 0xff000000000000 & q << 5 * 8 | q << 7 * 8;
#else
        return q;
#endif
    }
}
