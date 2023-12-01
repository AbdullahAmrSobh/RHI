#pragma once

#include <vk_mem_alloc.h>

#include <RHI/Resources.hpp>
#include <RHI/FrameScheduler.hpp>

namespace Vulkan
{
    VkImageLayout ConvertImageLayout(RHI::AttachmentUsage usage, RHI::AttachmentAccess access);

    VkPipelineStageFlags2 ConvertPipelineStageFlags(RHI::AttachmentUsage usage);

    VkPipelineStageFlags2 ConvertPipelineStageFlags(RHI::Flags<RHI::ShaderStage> stages);

    VkAccessFlags2 ConvertPipelineAccessFlags(RHI::Flags<RHI::AttachmentAccess> access);
    
} // namespace Vulkan