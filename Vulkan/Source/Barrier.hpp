#pragma once

#include <RHI/Common/Containers.h>
#include <RHI/RenderGraph.hpp>

#include "Common.hpp"

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    enum class BarrierType : uint32_t
    {
        Priloge,
        Epiloge,
        Count,
    };

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
    };

    struct PipelineBarriers
    {
        TL::Vector<VkMemoryBarrier2> memoryBarriers;
        TL::Vector<VkImageMemoryBarrier2> imageBarriers;
        TL::Vector<VkBufferMemoryBarrier2> bufferBarriers;
    };

    struct RenderingAttachmentFormats
    {
        TL::Vector<VkFormat> colorAttachments;
        VkFormat depthAttachmentInfo;
        VkFormat stencilAttachmentInfo;
    };

    struct ImageTransitionState
    {
        ImageStageAccess srcState;
        ImageStageAccess dstState;
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;

        inline bool CanSkip() const
        {
            return srcState == dstState && srcQueueFamilyIndex == dstQueueFamilyIndex;
        }
    };

    struct BufferTransitionState
    {
        BufferStageAccess srcState;
        BufferStageAccess dstState;
        uint32_t srcQueueFamilyIndex;
        uint32_t dstQueueFamilyIndex;
    };

    // clang-format off

    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_COLOR_ATTACHMENT_READ              = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_COLOR_ATTACHMENT_WRITE             = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_COLOR_ATTACHMENT_READ_WRITE        = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_DEPTH_TARGET_READ                  = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_DEPTH_TARGET_WRITE                 = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_DEPTH_TARGET_READ_WRITE            = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_SHADER_READ                        = VK_ACCESS_2_SHADER_READ_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_SHADER_WRITE                       = VK_ACCESS_2_SHADER_WRITE_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_SHADER_READ_WRITE                  = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;

    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_SAMPLED                            =  VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_SHADER_STORAGE_READ                = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_SHADER_STORAGE_WRITE               = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    inline static constexpr VkAccessFlags2   ACCESS_FLAGS_SHADER_STORAGE_READ_WRITE          = ACCESS_FLAGS_SHADER_STORAGE_READ | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_COLOR_READ               = { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, ACCESS_FLAGS_COLOR_ATTACHMENT_READ       };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_COLOR_WRITE              = { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, ACCESS_FLAGS_COLOR_ATTACHMENT_WRITE      };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_COLOR_READ_WRITE         = { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, ACCESS_FLAGS_COLOR_ATTACHMENT_READ_WRITE };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_DEPTH_READ               = { VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_READ           };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_DEPTH_WRITE              = { VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_WRITE          };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_DEPTH_READ_WRITE         = { VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_READ_WRITE     };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_STENCIL_READ             = { VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,       VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_READ           };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_STENCIL_WRITE            = { VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,       VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_WRITE          };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_STENCIL_READ_WRITE       = { VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,       VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_READ_WRITE     };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_DEPTH_STENCIL_READ       = { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_READ           };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_DEPTH_STENCIL_WRITE      = { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_WRITE          };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_DEPTH_STENCIL_READ_WRITE = { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    ACCESS_FLAGS_DEPTH_TARGET_READ_WRITE     };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_UNDEFINED                = { VK_IMAGE_LAYOUT_UNDEFINED,                        VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,             VK_ACCESS_2_NONE                         };
    inline static constexpr ImageStageAccess PIPELINE_IMAGE_BARRIER_PRESENT_SRC              = { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,          VK_ACCESS_2_NONE                         };

    // clang-format on

    inline static VkPipelineStageFlags2 GetPipelineStage(Flags<ShaderStage> shader)
    {
        VkPipelineStageFlags2 flags = {};

        if (shader & ShaderStage::Compute)
            return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

        if (shader & ShaderStage::Pixel)
            flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        if (shader & ShaderStage::Vertex)
            flags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;

        return flags;
    }

    inline static VkAccessFlags2 GetColorAccess(LoadStoreOperations loadStoreOps)
    {
        if (loadStoreOps.loadOperation == LoadOperation::Load && loadStoreOps.storeOperation == StoreOperation::Store)
            return ACCESS_FLAGS_COLOR_ATTACHMENT_READ_WRITE;
        else if (loadStoreOps.loadOperation == LoadOperation::Load && loadStoreOps.storeOperation != StoreOperation::Store)
            return ACCESS_FLAGS_COLOR_ATTACHMENT_READ;
        else if (loadStoreOps.loadOperation != LoadOperation::Load && loadStoreOps.storeOperation == StoreOperation::Store)
            return ACCESS_FLAGS_COLOR_ATTACHMENT_WRITE;
        else
            return VK_ACCESS_2_NONE;
    };

    inline static VkAccessFlags2 GetDepthStencilAccess(LoadStoreOperations loadStoreOps)
    {
        if (loadStoreOps.loadOperation == LoadOperation::Load && loadStoreOps.storeOperation == StoreOperation::Store)
            return ACCESS_FLAGS_DEPTH_TARGET_READ_WRITE;
        else if (loadStoreOps.loadOperation == LoadOperation::Load && loadStoreOps.storeOperation != StoreOperation::Store)
            return ACCESS_FLAGS_DEPTH_TARGET_READ;
        else if (loadStoreOps.loadOperation != LoadOperation::Load && loadStoreOps.storeOperation == StoreOperation::Store)
            return ACCESS_FLAGS_DEPTH_TARGET_WRITE;
        else
            return VK_ACCESS_2_NONE;
    };

    inline static VkAccessFlags2 GetAccess(Access accessFlags, bool isStorageResource)
    {
        switch (accessFlags)
        {
        case Access::None:      return VK_ACCESS_2_NONE;
        case Access::Read:      return isStorageResource ? ACCESS_FLAGS_SHADER_STORAGE_READ : ACCESS_FLAGS_SHADER_READ;
        case Access::Write:     return isStorageResource ? ACCESS_FLAGS_SHADER_STORAGE_WRITE : ACCESS_FLAGS_SHADER_WRITE;
        case Access::ReadWrite: return isStorageResource ? ACCESS_FLAGS_SHADER_STORAGE_READ_WRITE : ACCESS_FLAGS_SHADER_READ_WRITE;
        default:                RHI_UNREACHABLE(); return VK_ACCESS_2_NONE;
        }
    };

    inline static ImageStageAccess GetImageStageAccess(ImageUsage usage, Access access, Flags<ShaderStage> shaderStage, LoadStoreOperations loadStoreOperations)
    {
        // clang-format off
        switch (usage)
        {
        case ImageUsage::Color:           return { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, GetColorAccess(loadStoreOperations),        };
        case ImageUsage::Depth:           return { VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,         VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    GetDepthStencilAccess(loadStoreOperations), };
        case ImageUsage::Stencil:         return { VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,       VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    GetDepthStencilAccess(loadStoreOperations), };
        case ImageUsage::DepthStencil:    return { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    GetDepthStencilAccess(loadStoreOperations), };
        case ImageUsage::CopySrc:         return { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             VK_PIPELINE_STAGE_2_TRANSFER_BIT,                VK_ACCESS_2_TRANSFER_READ_BIT,              };
        case ImageUsage::CopyDst:         return { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             VK_PIPELINE_STAGE_2_TRANSFER_BIT,                VK_ACCESS_2_TRANSFER_WRITE_BIT,             };
        case ImageUsage::Sampled:  return { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         GetPipelineStage(shaderStage),                   GetAccess(access, false),                   };
        case ImageUsage::Storage: return { VK_IMAGE_LAYOUT_GENERAL,                          GetPipelineStage(shaderStage),                   GetAccess(access, true),                    };
        default:                          RHI_UNREACHABLE(); return {};
        }
        // clang-format on
    }

    inline static BufferStageAccess GetBufferStageAccess(BufferUsage usage, Access access, Flags<ShaderStage> shaderStage)
    {
        // clang-format off
        switch (usage)
        {
        case BufferUsage::Storage: return { GetPipelineStage(shaderStage),        GetAccess(access, true),       };
        case BufferUsage::Uniform: return { GetPipelineStage(shaderStage),        GetAccess(access, false),      };
        case BufferUsage::Vertex:  return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT, VK_ACCESS_2_SHADER_READ_BIT,   };
        case BufferUsage::Index:   return { VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,  VK_ACCESS_2_SHADER_READ_BIT,   };
        case BufferUsage::CopySrc: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,     VK_ACCESS_2_TRANSFER_READ_BIT, };
        case BufferUsage::CopyDst: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,     VK_ACCESS_2_TRANSFER_WRITE_BIT,};
        default:                   RHI_UNREACHABLE(); return {};
        }
        // clang-format on
    }

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

    inline static VkBufferMemoryBarrier2 CreateBufferBarrier(VkBuffer buffer, size_t offset, size_t range, BufferStageAccess src, BufferStageAccess dst, uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED, uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED)
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
        barrier.offset = offset;
        barrier.size = range;
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
        default:                       RHI_UNREACHABLE(); return {};
        };
    }

} // namespace RHI::Vulkan