#pragma once

#include <TL/Span.hpp>

namespace RHI
{
    enum QueueType : uint8_t
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    inline static constexpr uint32_t AsyncQueuesCount = QueueType::Count;
} // namespace RHI