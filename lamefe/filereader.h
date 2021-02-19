#pragma once

#include "exception.h"

namespace lamefe
{
    class FileReader
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

        virtual ~FileReader() {}

        virtual uint64_t fileSize() = 0;
        virtual Data read(uint64_t a_offset, size_t a_size) = 0;
        // Destroy buffer
        virtual void clear() = 0;
    };
}
