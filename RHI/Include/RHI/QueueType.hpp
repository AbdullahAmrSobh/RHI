#pragma once

#include <cstdint>

namespace RHI
{
    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };
}