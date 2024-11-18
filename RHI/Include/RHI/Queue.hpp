#pragma once

#include "RHI/PipelineAccess.hpp"

#include <TL/Span.hpp>

namespace RHI
{
    class CommandList;

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    struct SubmitInfo
    {
        uint64_t                     waitTimelineValue = 0;
        TL::Flags<PipelineStage>     waitPipelineStage = PipelineStage::None;
        TL::Span<CommandList* const> commandLists      = {};
        class Swapchain*             swapchainToWait   = nullptr;
        class Swapchain*             swapchainToSignal = nullptr;
    };
} // namespace RHI