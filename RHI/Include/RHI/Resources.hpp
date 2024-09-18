#pragma once

#include <TL/Flags.hpp>

namespace RHI
{
    enum class Access
    {
        None      = 0,
        Read      = 1 << 0,
        Write     = 1 << 1,
        ReadWrite = Read | Write,
    };

    TL_DEFINE_FLAG_OPERATORS(Access);
} // namespace RHI