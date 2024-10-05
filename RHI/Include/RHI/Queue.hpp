#pragma once

#include "RHI/Semaphore.hpp"

#include <TL/Span.hpp>

namespace RHI
{
    class Pass;
    class CommandList;
    class Fence;

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    struct SubmitInfo
    {
        TL::Span<const SemaphoreSubmitInfo> waitSemaphores;
        TL::Span<CommandList* const>        commandLists;
        TL::Span<const SemaphoreSubmitInfo> signalSemaphores;
    };

    class Queue
    {
    public:
        virtual ~Queue() = default;

        virtual void BeginLabel(const char* name, float color[4]) = 0;

        virtual void EndLabel() = 0;

        virtual void Submit(TL::Span<const SubmitInfo> submitInfos, Fence* fence) = 0;
    };
} // namespace RHI