#pragma once

#include <iostream>
#include <sstream>

//#define LAMEFE_WIDE 1

#ifdef LAMEFE_WIDE
#   define LAMEFE_CHAR wchar_t
#   define LAMEFE_STRING std::wstring
#   define LAMEFE_OSTREAM std::wostream
#   define LAMEFE_SSTREAM std::wstringstream
#   define LAMEFE_C(c) L ## c
#   define LAMEFE_S(s) L ## s
#else
#   define LAMEFE_CHAR char
#   define LAMEFE_STRING std::string
#   define LAMEFE_OSTREAM std::ostream
#   define LAMEFE_SSTREAM std::stringstream
#   define LAMEFE_C(c) c
#   define LAMEFE_S(s) s
#endif

namespace lamefe
{
    using Char = LAMEFE_CHAR;
    using String = LAMEFE_STRING;
    using OStream = LAMEFE_OSTREAM;
    using SStream = LAMEFE_SSTREAM;
}
