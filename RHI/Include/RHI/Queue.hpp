#pragma once

#include <RHI/Definitions.hpp>

#include <TL/Span.hpp>

namespace RHI
{
    class CommandList;
    class Fence;

    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Count,
    };

    class Queue
    {
    public:
        virtual ~Queue() = default;

        virtual void BeginLabel(const char* name, ColorValue<float> color) = 0;

        virtual void EndLabel() = 0;

        virtual void Submit(TL::Span<CommandList* const> commandLists, Fence* fence) = 0;
    };
} // namespace RHI