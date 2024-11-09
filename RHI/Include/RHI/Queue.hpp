#pragma once

#include "RHI/PipelineAccess.hpp"

#include <TL/Span.hpp>

namespace RHI
{
    class Pass;
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

    class Queue
    {
    public:
        virtual ~Queue()                                              = default;

        virtual void     BeginLabel(const char* name, float color[4]) = 0;

        virtual void     EndLabel()                                   = 0;

        virtual uint64_t Submit(const SubmitInfo& submitInfo)         = 0;
    };
} // namespace RHI