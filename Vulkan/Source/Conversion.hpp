#pragma once

#include <vk_mem_alloc.h>

#include <RHI/Resources.hpp>
#include <RHI/FrameScheduler.hpp>

namespace Vulkan
{

    inline static bool IsWriteAccess(RHI::ShaderAccess access)
    {
        return (access & RHI::ShaderAccess::Write) == RHI::ShaderAccess::Write;
    }

    inline static bool IsRenderTarget(RHI::ImageUsage usage)
    {
        return usage == RHI::ImageUsage::Color | usage == RHI::ImageUsage::Depth | usage == RHI::ImageUsage::Stencil;
    }

    inline static VkAttachmentLoadOp ConvertLoadOp(RHI::ImageLoadOperation op)
    {
        switch (op)
        {
        case RHI::ImageLoadOperation::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case RHI::ImageLoadOperation::Load:     return VK_ATTACHMENT_LOAD_OP_LOAD;
        case RHI::ImageLoadOperation::Discard:  return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:                                RHI_UNREACHABLE(); return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    inline static VkAttachmentStoreOp ConvertStoreOp(RHI::ImageStoreOperation op)
    {
        switch (op)
        {
        case RHI::ImageStoreOperation::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case RHI::ImageStoreOperation::Store:    return VK_ATTACHMENT_STORE_OP_STORE;
        case RHI::ImageStoreOperation::Discard:  return VK_ATTACHMENT_STORE_OP_NONE;
        default:                                 RHI_UNREACHABLE(); return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
        }
    }

    inline static VkImageLayout ConvertImageLayout(RHI::ImageUsage usage, RHI::ShaderAccess access)
    {
        if (usage == RHI::ImageUsage::StorageImage && IsWriteAccess(access))
        {
            return VK_IMAGE_LAYOUT_GENERAL;
        }

        switch (usage)
        {
        case RHI::ImageUsage::SampledImage: return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
        case RHI::ImageUsage::Color:        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case RHI::ImageUsage::Depth:        return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL_KHR;
        case RHI::ImageUsage::Stencil:      return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        case RHI::ImageUsage::Copy:         return IsWriteAccess(access) ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        default:
            {
                RHI_UNREACHABLE();
            }
        }

        return VK_IMAGE_LAYOUT_MAX_ENUM;
    }

    inline static VkAccessFlags2 ConvertPipelineAccess(RHI::ImageUsage usage, RHI::ShaderAccess access, bool access2)
    {
        switch (usage)
        {
        case RHI::ImageUsage::SampledImage: return VK_IMAGE_USAGE_SAMPLED_BIT;
        case RHI::ImageUsage::StorageImage: return IsWriteAccess(access) ? VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
        case RHI::ImageUsage::Color:        return access2 ? VK_ACCESS_2_NONE : VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        case RHI::ImageUsage::Depth:        return access2 ? VK_ACCESS_2_NONE : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case RHI::ImageUsage::Stencil:      return access2 ? VK_ACCESS_2_NONE : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case RHI::ImageUsage::Copy:         return IsWriteAccess(access) ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;
        default:                            RHI_UNREACHABLE(); return {};
        }

        return VK_ACCESS_2_NONE;
    }

    inline static VkAccessFlags2 ConvertPipelineAccess(RHI::BufferUsage usage, RHI::ShaderAccess access)
    {
        switch (usage)
        {
        case RHI::BufferUsage::None:
        case RHI::BufferUsage::Storage: return IsWriteAccess(access) ? VK_ACCESS_2_SHADER_STORAGE_READ_BIT : VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        case RHI::BufferUsage::Uniform: return VK_ACCESS_2_UNIFORM_READ_BIT;
        case RHI::BufferUsage::Vertex:  return VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
        case RHI::BufferUsage::Index:   return VK_ACCESS_2_INDEX_READ_BIT;
        case RHI::BufferUsage::Copy:    return IsWriteAccess(access) ? VK_ACCESS_2_TRANSFER_READ_BIT : VK_ACCESS_2_TRANSFER_WRITE_BIT;
        default:                        RHI_UNREACHABLE();
        }

        return VK_ACCESS_2_NONE;
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(RHI::ShaderStage stage)
    {
        switch (stage)
        {
        case RHI::ShaderStage::Vertex:  return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        case RHI::ShaderStage::Pixel:   return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        case RHI::ShaderStage::Compute: return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        }
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(RHI::ImageUsage usage, RHI::ShaderStage stage)
    {
        switch (usage)
        {
        case RHI::ImageUsage::None:         return VK_PIPELINE_STAGE_2_NONE;
        case RHI::ImageUsage::SampledImage: return ConvertPipelineStageFlags(stage);
        case RHI::ImageUsage::StorageImage: return ConvertPipelineStageFlags(stage);
        case RHI::ImageUsage::Color:        return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case RHI::ImageUsage::Depth:        return VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        case RHI::ImageUsage::Stencil:      return VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        case RHI::ImageUsage::Copy:         return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        default:                            RHI_UNREACHABLE();
        }

        return VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(RHI::BufferUsage usage, RHI::ShaderStage stage)
    {
        switch (usage)
        {
        case RHI::BufferUsage::None:    return VK_PIPELINE_STAGE_2_NONE;
        case RHI::BufferUsage::Storage: return ConvertPipelineStageFlags(stage);
        case RHI::BufferUsage::Uniform: return ConvertPipelineStageFlags(stage);
        case RHI::BufferUsage::Vertex:  return VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
        case RHI::BufferUsage::Index:   return VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
        case RHI::BufferUsage::Copy:    return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        default:                        RHI_UNREACHABLE();
        }

        return VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;
    }

} // namespace Vulkan