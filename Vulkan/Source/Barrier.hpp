#pragma once

#include <RHI/RenderGraph.hpp>

#include <TL/Assert.hpp>

#include "Common.hpp"
#include "Device.hpp"

#include <TL/Memory.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    struct ImageStageAccess
    {
        VkImageLayout layout;
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;
        uint32_t queueFamilyIndex;

        inline bool operator==(const ImageStageAccess& other) const
        {
            return stage == other.stage && access == other.access && layout == other.layout && queueFamilyIndex == other.queueFamilyIndex;
        }
    };

    struct BufferStageAccess
    {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;
        uint32_t queueFamilyIndex;

        inline bool operator==(const BufferStageAccess& other) const
        {
            return stage == other.stage && access == other.access && queueFamilyIndex == other.queueFamilyIndex;
        }
    };

    inline static TL::Flags<Access> ConvertLoadStoreOperationsToAccess(LoadStoreOperations operations)
    {
        TL::Flags<Access> flags = Access::None;
        if (operations.loadOperation == LoadOperation::Load) flags |= Access::Read;
        if (operations.storeOperation == StoreOperation::Store) flags |= Access::Write;
        return flags;
    }

    inline static VkPipelineStageFlags2 ConvertPipelineStageFlags(TL::Flags<PipelineStage> pipelineStages)
    {
        VkPipelineStageFlags2 stageFlags = {};
        if (pipelineStages & PipelineStage::TopOfPipe) stageFlags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        if (pipelineStages & PipelineStage::DrawIndirect) stageFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        if (pipelineStages & PipelineStage::VertexInput) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
        if (pipelineStages & PipelineStage::VertexShader) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        if (pipelineStages & PipelineStage::TessellationControlShader) stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
        if (pipelineStages & PipelineStage::TessellationEvaluationShader) stageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
        if (pipelineStages & PipelineStage::PixelShader) stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        if (pipelineStages & PipelineStage::EarlyFragmentTests) stageFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
        if (pipelineStages & PipelineStage::LateFragmentTests) stageFlags |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        if (pipelineStages & PipelineStage::ColorAttachmentOutput) stageFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        if (pipelineStages & PipelineStage::ComputeShader) stageFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        if (pipelineStages & PipelineStage::Transfer) stageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        if (pipelineStages & PipelineStage::BottomOfPipe) stageFlags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
        if (pipelineStages & PipelineStage::Host) stageFlags |= VK_PIPELINE_STAGE_2_HOST_BIT;
        if (pipelineStages & PipelineStage::AllGraphics) stageFlags |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        if (pipelineStages & PipelineStage::AllCommands) stageFlags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        if (pipelineStages & PipelineStage::Copy) stageFlags |= VK_PIPELINE_STAGE_2_COPY_BIT;
        if (pipelineStages & PipelineStage::Resolve) stageFlags |= VK_PIPELINE_STAGE_2_RESOLVE_BIT;
        if (pipelineStages & PipelineStage::Blit) stageFlags |= VK_PIPELINE_STAGE_2_BLIT_BIT;
        if (pipelineStages & PipelineStage::Clear) stageFlags |= VK_PIPELINE_STAGE_2_CLEAR_BIT;
        if (pipelineStages & PipelineStage::IndexInput) stageFlags |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
        if (pipelineStages & PipelineStage::VertexAttributeInput) stageFlags |= VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
        if (pipelineStages & PipelineStage::PreRasterizationShaders) stageFlags |= VK_PIPELINE_STAGE_2_PRE_RASTERIZATION_SHADERS_BIT;
        if (pipelineStages & PipelineStage::TransformFeedback) stageFlags |= VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT;
        if (pipelineStages & PipelineStage::ConditionalRendering) stageFlags |= VK_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT;
        if (pipelineStages & PipelineStage::FragmentShadingRateAttachment) stageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
        if (pipelineStages & PipelineStage::AccelerationStructureBuild) stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
        if (pipelineStages & PipelineStage::RayTracingShader) stageFlags |= VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR;
        if (pipelineStages & PipelineStage::TaskShader) stageFlags |= VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
        if (pipelineStages & PipelineStage::MeshShader) stageFlags |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
        if (pipelineStages & PipelineStage::AccelerationStructureCopy) stageFlags |= VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_COPY_BIT_KHR;
        return stageFlags;
    }

    inline static VkAccessFlags2 GetAccessFlagsForPassAttachment(const RGImagePassAccess& imageAttachment)
    {
        switch (imageAttachment.usage)
        {
        case ImageUsage::None: return {};
        case ImageUsage::ShaderResource:
            {
                if (imageAttachment.pipelineAccess == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else if (imageAttachment.pipelineAccess == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT;
                }
                else if (imageAttachment.pipelineAccess == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case ImageUsage::StorageResource:
            {
                if (imageAttachment.pipelineAccess == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else if (imageAttachment.pipelineAccess == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                }
                else if (imageAttachment.pipelineAccess == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case ImageUsage::Color:
            {
                auto access = ConvertLoadStoreOperationsToAccess(imageAttachment.loadStoreOperation);
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case ImageUsage::Depth:
        case ImageUsage::Stencil:
        case ImageUsage::DepthStencil:
            {
                auto access = ConvertLoadStoreOperationsToAccess(imageAttachment.loadStoreOperation);
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case ImageUsage::CopySrc:
        case ImageUsage::CopyDst:
            {
                if (imageAttachment.pipelineAccess == Access::ReadWrite)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else if (imageAttachment.pipelineAccess == Access::Read)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT;
                }
                else if (imageAttachment.pipelineAccess == Access::Write)
                {
                    return VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        default: TL_UNREACHABLE(); return {};
        };
    }

    inline static VkImageLayout GetImageAttachmentLayout(const RGImagePassAccess& imageAttachment)
    {
        auto imageAspect = imageAttachment.viewInfo.subresources.imageAspects;
        switch (imageAttachment.usage)
        {
        case ImageUsage::Color: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageUsage::DepthStencil:
            {
                auto access = ConvertLoadStoreOperationsToAccess(imageAttachment.loadStoreOperation);
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

                TL_UNREACHABLE();
                return VK_IMAGE_LAYOUT_GENERAL;
            }
        case ImageUsage::ShaderResource:
            {
                bool isReadOnly = imageAttachment.pipelineAccess == Access::Read;

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

    inline static ImageStageAccess GetImageStageAccess(const RGImagePassAccess& imageAttachment)
    {
        return {
            .layout = GetImageAttachmentLayout(imageAttachment),
            .stage = ConvertPipelineStageFlags(imageAttachment.pipelineStages),
            .access = GetAccessFlagsForPassAttachment(imageAttachment),
            .queueFamilyIndex = 0,
        };
    }

    inline static VkImageMemoryBarrier2 CreateImageBarrier(VkImage image, VkImageSubresourceRange subresourceRange, ImageStageAccess src, ImageStageAccess dst)
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
        barrier.srcQueueFamilyIndex = src.queueFamilyIndex;
        barrier.dstQueueFamilyIndex = dst.queueFamilyIndex;
        barrier.image = image;
        barrier.subresourceRange = subresourceRange;
        return barrier;
    }

    inline static VkBufferMemoryBarrier2 CreateBufferBarrier(VkBuffer buffer, BufferSubregion subregion, BufferStageAccess src, BufferStageAccess dst)
    {
        VkBufferMemoryBarrier2 barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
        barrier.pNext = nullptr;
        barrier.srcStageMask = src.stage;
        barrier.srcAccessMask = src.access;
        barrier.dstStageMask = dst.stage;
        barrier.dstAccessMask = dst.access;
        barrier.srcQueueFamilyIndex = src.queueFamilyIndex;
        barrier.dstQueueFamilyIndex = dst.queueFamilyIndex;
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

    inline static VkRenderingAttachmentInfo CreateRenderingAttachment(VkImageView attachmentView, ImageUsage usage, LoadStoreOperations loadStoreOperations)
    {
        switch (usage)
        {
        case ImageUsage::Color:        return CreateColorAttachment(attachmentView, loadStoreOperations);
        case ImageUsage::Depth:        return CreateDepthAttachment(attachmentView, loadStoreOperations);
        case ImageUsage::Stencil:      return CreateStencilAttachment(attachmentView, loadStoreOperations);
        case ImageUsage::DepthStencil: TL_UNREACHABLE(); return {};
        default:                       TL_UNREACHABLE(); return {};
        };
    }
} // namespace RHI::Vulkan