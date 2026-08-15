#pragma once

#include <RHI/RHI.h>

namespace RHI::WebGPU
{
    inline constexpr size_t MaxCommandListsPerPool = 64;
    inline constexpr size_t MaxBindingsPerGroup = 32;
    inline constexpr size_t MaxBindGroupsPerPipeline = 8;
    inline constexpr size_t MaxVertexBuffers = 16;
    inline constexpr size_t MaxVertexAttributesPerBuffer = 16;
    inline constexpr size_t MaxColorAttachments = 8;
    inline constexpr size_t MaxSubmittedCommandLists = 64;
} // namespace RHI::WebGPU
