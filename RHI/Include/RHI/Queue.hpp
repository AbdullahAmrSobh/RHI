#pragma once

#include "RHI/PipelineAccess.hpp"

#include <TL/Span.hpp>

namespace RHI
{
    class CommandList;

    enum QueueType : uint8_t
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };
} // namespace RHI