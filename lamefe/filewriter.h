#pragma once

#include "exception.h"

namespace lamefe
{
    class FileWriter
    {
    public:
        class Exception: public lamefe::Exception
        {
        public:
            Exception(const std::string &a_what): lamefe::Exception(a_what) {}
        };

        struct Data
        {
            uint8_t *begin = nullptr, *end = nullptr;
            Data(uint8_t *a_begin, uint8_t *a_end): begin(a_begin), end(a_end) {}
            size_t size() const { return end - begin; }
        };

        virtual ~FileWriter() {}

        // flush shall be called per write call (buffer is invalidated)
        virtual Data write(uint64_t a_offset, size_t a_size) = 0;
        virtual void flush(size_t a_size, bool a_flushUnderlying = false) = 0;

        // Destroy buffer
        virtual void clear() = 0;
    };
}
