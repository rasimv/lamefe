#pragma once

#include <lame.h>

namespace lamefe
{
    // Helper RAII-struct
    struct LameContext
    {
        lame_t gf = nullptr;

        LameContext(): gf(::lame_init()) {}

        ~LameContext() { ::lame_close(gf); }
    };
}
