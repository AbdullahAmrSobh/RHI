#pragma once

#include "RHI/Export.hpp"

namespace RHI
{
    enum class QueueType
    {
        None,
        Graphics,
        Compute,
        Transfer,
    };

    class RHI_EXPORT Queue
    {
    public:
        virtual void Submit();

        virtual void Present();

        virtual void BeginLabel();

        virtual void EndLabel();

        virtual void BindSparse();

    };
}