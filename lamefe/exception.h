#pragma once

#include <string>

namespace lamefe
{
    class Exception: public std::exception
    {
    public:
        Exception(const std::string &a_what): m_what(a_what) {}

        const char *what() const noexcept override { return m_what.c_str(); }

    private:
        std::string m_what;
    };
}
