#pragma once

#include "RHI/Handle.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{
    RHI_DECLARE_OPAQUE_RESOURCE(Semaphore);

    struct SemaphoreCreateInfo
    {
        const char* name;
        bool        timeline;
    };

    struct SemaphoreSubmitInfo
    {
        uint64_t                 value;
        TL::Flags<PipelineStage> pipelineStages;
        Handle<Semaphore>        semaphore;
    };

} // namespace RHI