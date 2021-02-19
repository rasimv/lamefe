#pragma once

#include "wide.h"
#include "exception.h"
#include <filesystem>
#include "settings.h"

namespace lamefe
{
    class EncoderImpl;

    class Encoder
    {
    public:
        class Exception: public lamefe::Exception
        {
        public:
            Exception(const std::string &a_what): lamefe::Exception(a_what) {}
        };

        Encoder(OStream &a_log);
        Encoder(const Encoder &) = delete;
        Encoder(Encoder &&a) noexcept { swap(a, *this); }

        Encoder &operator =(const Encoder &) = delete;
        Encoder &operator =(Encoder &&a) noexcept { swap(a, *this); return *this; }

        ~Encoder();

        void set(Settings a);
        void list(std::filesystem::path a_dir);
        void run();

        friend void swap(Encoder &o, Encoder &a) noexcept { std::swap(o.m_impl, a.m_impl); }

    private:
        EncoderImpl *m_impl = nullptr;
    };
}
