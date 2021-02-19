#pragma once

#include <cstddef>
#include <Windows.h>

namespace lamefe
{
    inline size_t granularity()
    {
        SYSTEM_INFO l_si;
        ::ZeroMemory(&l_si, sizeof(l_si));
        ::GetSystemInfo(&l_si);
        return l_si.dwAllocationGranularity;
    }
}
