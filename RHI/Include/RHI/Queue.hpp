#pragma once

#include <TL/Span.hpp>

namespace RHI
{
    enum class QueueType : uint8_t
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    inline static constexpr uint32_t AsyncQueuesCount = (uint32_t)QueueType::Count;
} // namespace RHI