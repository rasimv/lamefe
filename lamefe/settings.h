#pragma once

#include <optional>

namespace lamefe
{
    // Encoding settings
    struct Settings
    {
        enum Mode { SMCBR, SMABR, SMVBR };
        std::optional<unsigned> quality;
        std::optional<Mode> mode;
        std::optional<unsigned> value;
    };
}
