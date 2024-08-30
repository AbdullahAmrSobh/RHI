#pragma once

#include <RHI/RenderGraph.hpp>

#include <TL/Assert.hpp>

#include "Common.hpp"
#include "Context.hpp"

#include <TL/Memory.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    struct ImageStageAccess
    {
        VkImageLayout layout;
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;

        inline bool operator==(const ImageStageAccess& other) const
        {
            return stage == other.stage && access == other.access && layout == other.layout;
        }
    };

    struct BufferStageAccess
    {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;

        inline bool operator==(const BufferStageAccess& other) const
        {
            return stage == other.stage && access == other.access;
        }
    };

    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_UNDEFINED = { VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_PRESENT_SRC = { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_TRANSFER_SRC = { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_TRANSFER_DST = { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_SHADER_READ = { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT };

    // buffer stage accesses
    inline static constexpr BufferStageAccess PIPELINE_BUFFER_BARRIER_TOP = { VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE };
    inline static constexpr BufferStageAccess PIPELINE_BUFFER_BARRIER_BOTTOM = { VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE };
    inline static constexpr BufferStageAccess PIPELINE_BUFFER_BARRIER_TRANSFER_SRC = { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT };
    inline static constexpr BufferStageAccess PIPELINE_BUFFER_BARRIER_TRANSFER_DST = { VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT };
    inline static constexpr BufferStageAccess PIPELINE_BUFFER_BARRIER_SHADER_READ = { VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT };

    inline static VkPipelineStageFlags2 GetPipelineStageFromShaderStages(TL::Flags<ShaderStage> shadeStages)
    {
        VkPipelineStageFlags2 stageFlags = {};

        if (shadeStages & ShaderStage::Compute)
            return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

        if (shadeStages & ShaderStage::Pixel)
            stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

        if (shadeStages & ShaderStage::Vertex)
            stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;

        return stageFlags;
    }

    inline static Access ConvertLoadStoreOperationsToAcces(LoadStoreOperations operations)
    {
        uint64_t flags{};
        if (operations.loadOperation == LoadOperation::Load)
            flags |= uint64_t(Access::Read);
        if (operations.storeOperation == StoreOperation::Store)
            flags |= uint64_t(Access::Write);
        return Access(flags);
    }

    inline static VkAccessFlags2 GetAccessFlagsForColorAttachments(LoadStoreOperations operations)
    {
        if (operations.loadOperation == LoadOperation::Load && operations.storeOperation == StoreOperation::Store)
            return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        else if (operations.loadOperation == LoadOperation::Load && operations.storeOperation != StoreOperation::Store)
            return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
        else if (operations.loadOperation != LoadOperation::Load && operations.storeOperation == StoreOperation::Store)
            return VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        else
            return VK_ACCESS_2_NONE;
    };

    inline static VkAccessFlags2 GetAccessFlagsForDepthAttachment(LoadStoreOperations operations)
    {
        if (operations.loadOperation == LoadOperation::Load && operations.storeOperation == StoreOperation::Store)
            return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        else if (operations.loadOperation == LoadOperation::Load && operations.storeOperation != StoreOperation::Store)
            return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        else if (operations.loadOperation != LoadOperation::Load && operations.storeOperation == StoreOperation::Store)
            return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        else
            return VK_ACCESS_2_NONE;
    };

    inline static VkAccessFlags2 GetAccessFlagsForHost(Access access)
    {
        switch (access)
        {
        case Access::None:      return VK_ACCESS_2_NONE;
        case Access::Read:      return VK_ACCESS_2_HOST_READ_BIT;
        case Access::Write:     return VK_ACCESS_2_HOST_WRITE_BIT;
        case Access::ReadWrite: return VK_ACCESS_2_HOST_READ_BIT | VK_ACCESS_2_HOST_WRITE_BIT;
        default:                TL_UNREACHABLE(); return VK_ACCESS_2_NONE;
        }
    };

    inline static VkAccessFlags2 GetAccessFlagsForTransfer(Access access)
    {
        switch (access)
        {
        case Access::None:      return VK_ACCESS_2_NONE;
        case Access::Read:      return VK_ACCESS_2_TRANSFER_READ_BIT;
        case Access::Write:     return VK_ACCESS_2_TRANSFER_WRITE_BIT;
        case Access::ReadWrite: return VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
        default:                TL_UNREACHABLE(); return VK_ACCESS_2_NONE;
        }
    };

    inline static VkAccessFlags2 GetAccessFlagsForMemory(Access access)
    {
        switch (access)
        {
        case Access::None:      return VK_ACCESS_2_NONE;
        case Access::Read:      return VK_ACCESS_2_MEMORY_READ_BIT;
        case Access::Write:     return VK_ACCESS_2_MEMORY_WRITE_BIT;
        case Access::ReadWrite: return VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
        default:                TL_UNREACHABLE(); return VK_ACCESS_2_NONE;
        }
    };

    inline static VkAccessFlags2 GetAccessFlagsForShaderResource(Access access)
    {
        switch (access)
        {
        case Access::None:      return VK_ACCESS_2_NONE;
        case Access::Read:      return VK_ACCESS_2_SHADER_READ_BIT;
        case Access::Write:     return VK_ACCESS_2_SHADER_WRITE_BIT;
        case Access::ReadWrite: return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
        default:                TL_UNREACHABLE(); return VK_ACCESS_2_NONE;
        }
    }

    inline static VkAccessFlags2 GetAccessFlagsForShaderStorageResource(Access access)
    {
        switch (access)
        {
        case Access::None:      return VK_ACCESS_2_NONE;
        case Access::Read:      return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        case Access::Write:     return VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        case Access::ReadWrite: return VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        default:                TL_UNREACHABLE(); return VK_ACCESS_2_NONE;
        }
    }

    inline static VkAccessFlags2 GetAccessFlagsForPassAttachment(const ImagePassAttachment& imageAttachment)
    {
        switch (imageAttachment.usage)
        {
        case ImageUsage::None:            return {};
        case ImageUsage::ShaderResource:  return GetAccessFlagsForShaderResource(imageAttachment.access);
        case ImageUsage::StorageResource: return GetAccessFlagsForShaderStorageResource(imageAttachment.access);
        case ImageUsage::Color:           return GetAccessFlagsForColorAttachments(imageAttachment.loadStoreOperation);
        case ImageUsage::Depth:
        case ImageUsage::Stencil:
        case ImageUsage::DepthStencil:    return GetAccessFlagsForDepthAttachment(imageAttachment.loadStoreOperation);
        case ImageUsage::CopySrc:
        case ImageUsage::CopyDst:         return GetAccessFlagsForTransfer(imageAttachment.access);
        default:                          TL_UNREACHABLE(); return {};
        };
    }

    inline static VkPipelineStageFlags2 GetPipelineStageFlagsForPassAttachment(const ImagePassAttachment& imageAttachment)
    {
        switch (imageAttachment.usage)
        {
        case ImageUsage::None:            return {};
        case ImageUsage::ShaderResource:  return GetPipelineStageFromShaderStages(imageAttachment.stages);
        case ImageUsage::StorageResource: return GetPipelineStageFromShaderStages(imageAttachment.stages);
        case ImageUsage::Color:           return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case ImageUsage::Depth:
        case ImageUsage::Stencil:
        case ImageUsage::DepthStencil:    return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        case ImageUsage::CopySrc:
        case ImageUsage::CopyDst:         return VK_PIPELINE_STAGE_2_COPY_BIT;
        default:                          TL_UNREACHABLE(); return {};
        };
    }

    inline static VkImageLayout GetImageAttachmentLayout(const ImagePassAttachment& imageAttachment)
    {
        auto imageAspect = imageAttachment.viewInfo.subresources.imageAspects;
        switch (imageAttachment.usage)
        {
        case ImageUsage::Color: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageUsage::DepthStencil:
            {
                auto access = ConvertLoadStoreOperationsToAcces(imageAttachment.loadStoreOperation);
                if (imageAspect & ImageAspect::DepthStencil)
                {
                    return access == Access::Read ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else if (imageAspect & ImageAspect::Depth)
                {
                    return access == Access::Read ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                }
                else if (imageAspect & ImageAspect::Stencil)
                {
                    return access == Access::Read ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                }

                return VK_IMAGE_LAYOUT_GENERAL;
            }
        case ImageUsage::ShaderResource:
            {
                bool isReadOnly = imageAttachment.access == Access::Read;

                if (imageAspect & ImageAspect::Color)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                }
                else if (imageAspect & ImageAspect::DepthStencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else if (imageAspect & ImageAspect::Depth)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                }
                else if (imageAspect & ImageAspect::Stencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                }
                TL_UNREACHABLE();
                return VK_IMAGE_LAYOUT_GENERAL;
            }
        case ImageUsage::CopySrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageUsage::CopyDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        default:                  return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    inline static ImageStageAccess GetImageStageAccess(const ImagePassAttachment& imageAttachment)
    {
        return {
            .layout = GetImageAttachmentLayout(imageAttachment),
            .stage = GetPipelineStageFlagsForPassAttachment(imageAttachment),
            .access = GetAccessFlagsForPassAttachment(imageAttachment),
        };
    }

    // inline static BufferStageAccess GetBufferStageAccess(const BufferPassAttachment& imageAttachment)
    // {
    //     return {
    //         .stage = GetPipelineStageFlagsFor_(imageAttachment),
    //         .access = GetAccessFlagsForPassAttachment(imageAttachment),
    //     };
    // }

    inline static VkImageMemoryBarrier2 CreateImageBarrier(VkImage image, VkImageSubresourceRange subresourceRange, ImageStageAccess src, ImageStageAccess dst, uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED)
    {
        VkImageMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcStageMask = src.stage;
        barrier.srcAccessMask = src.access;
        barrier.dstStageMask = dst.stage;
        barrier.dstAccessMask = dst.access;
        barrier.oldLayout = src.layout;
        barrier.newLayout = dst.layout;
        barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        barrier.image = image;
        barrier.subresourceRange = subresourceRange;
        return barrier;
    }

    inline static VkBufferMemoryBarrier2 CreateBufferBarrier(VkBuffer buffer, BufferSubregion subregion, BufferStageAccess src, BufferStageAccess dst, uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED)
    {
        VkBufferMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcStageMask = src.stage;
        barrier.srcAccessMask = src.access;
        barrier.dstStageMask = dst.stage;
        barrier.dstAccessMask = dst.access;
        barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
        barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
        barrier.buffer = buffer;
        barrier.offset = subregion.offset;
        barrier.size = subregion.size;
        return barrier;
    }

    inline static VkRenderingAttachmentInfo CreateColorAttachment(VkImageView attachmentView, LoadStoreOperations loadStoreOperations)
    {
        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = attachmentView;
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachmentInfo.loadOp = ConvertLoadOp(loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(loadStoreOperations.storeOperation);
        attachmentInfo.clearValue.color = ConvertClearValue(loadStoreOperations.clearValue);
        return attachmentInfo;
    }

    inline static VkRenderingAttachmentInfo CreateColorResolveAttachment(VkImageView colorView, VkImageView resolveView, LoadStoreOperations loadStoreOperations, VkResolveModeFlagBits resolveMode)
    {
        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = colorView;
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        attachmentInfo.resolveMode = resolveMode;
        attachmentInfo.resolveImageView = resolveView;
        attachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        attachmentInfo.loadOp = ConvertLoadOp(loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(loadStoreOperations.storeOperation);
        attachmentInfo.clearValue.color = ConvertClearValue(loadStoreOperations.clearValue);
        return attachmentInfo;
    }

    inline static VkRenderingAttachmentInfo CreateDepthAttachment(VkImageView depthView, LoadStoreOperations loadStoreOperations)
    {
        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = depthView;
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        attachmentInfo.loadOp = ConvertLoadOp(loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(loadStoreOperations.storeOperation);
        attachmentInfo.clearValue.depthStencil = ConvertDepthStencilValue(loadStoreOperations.clearValue.depthStencil);
        return attachmentInfo;
    }

    inline static VkRenderingAttachmentInfo CreateStencilAttachment(VkImageView depthView, LoadStoreOperations loadStoreOperations)
    {
        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = depthView;
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        attachmentInfo.loadOp = ConvertLoadOp(loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(loadStoreOperations.storeOperation);
        attachmentInfo.clearValue.depthStencil = ConvertDepthStencilValue(loadStoreOperations.clearValue.depthStencil);
        return attachmentInfo;
    }

    inline static VkRenderingAttachmentInfo CreateDepthStencilAttachment(VkImageView depthView, LoadStoreOperations loadStoreOperations)
    {
        VkRenderingAttachmentInfo attachmentInfo{};
        attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachmentInfo.pNext = nullptr;
        attachmentInfo.imageView = depthView;
        attachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachmentInfo.loadOp = ConvertLoadOp(loadStoreOperations.loadOperation);
        attachmentInfo.storeOp = ConvertStoreOp(loadStoreOperations.storeOperation);
        attachmentInfo.clearValue.depthStencil = ConvertDepthStencilValue(loadStoreOperations.clearValue.depthStencil);
        return attachmentInfo;
    }

    inline static VkRenderingAttachmentInfo CreateRenderingAttachment(VkImageView attachmentView, ImageUsage usage, LoadStoreOperations loadStoreOperations)
    {
        switch (usage)
        {
        case ImageUsage::Color:        return CreateColorAttachment(attachmentView, loadStoreOperations);
        case ImageUsage::Depth:        return CreateDepthAttachment(attachmentView, loadStoreOperations);
        case ImageUsage::Stencil:      return CreateStencilAttachment(attachmentView, loadStoreOperations);
        case ImageUsage::DepthStencil: return CreateDepthStencilAttachment(attachmentView, loadStoreOperations);
        default:                       TL_UNREACHABLE(); return {};
        };
    }
} // namespace RHI::Vulkan