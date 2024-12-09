#include "RenderGraph.hpp"

#include <vulkan/vulkan.h>

#include <tracy/Tracy.hpp>

#include "CommandList.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

namespace RHI::Vulkan
{
    inline static ColorValue<float> QueueTypeToColor(QueueType queueType)
    {
        switch (queueType)
        {
        case QueueType::Graphics: return {0.8f, 0.2f, 0.2f, 1.0f};
        case QueueType::Compute:  return {0.2f, 0.2f, 0.8f, 1.0f};
        case QueueType::Transfer: return {0.2f, 0.8f, 0.2f, 1.0f};
        case QueueType::Count:    TL_UNREACHABLE(); return {};
        }
        TL_UNREACHABLE();
        return {};
    }

    inline static VkAccessFlags2 GetAccessFlags2(ImageUsage usage, TL::Flags<Access> access)
    {
        VkAccessFlags2 result = VK_ACCESS_2_NONE;
        switch (usage)
        {
        case ImageUsage::ShaderResource:
            if (access & Access::Read) result |= VK_ACCESS_2_SHADER_READ_BIT;
            TL_ASSERT((access & Access::Write) == Access::None, "ImageUsage::ShaderResource can't have write access");
            break;
        case ImageUsage::StorageResource:
            if (access & Access::Read) result |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            break;
        case ImageUsage::CopySrc:
        case ImageUsage::CopyDst:
            if (access & Access::Read) result |= VK_ACCESS_2_TRANSFER_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case ImageUsage::Color:
            if (access & Access::Read) result |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case ImageUsage::Depth:
        case ImageUsage::Stencil:
        case ImageUsage::DepthStencil:
            if (access & Access::Read) result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case RHI::ImageUsage::_SwapchainPresent:
            return VK_ACCESS_2_NONE;
        default: break;
        };
        TL_ASSERT(result != VK_ACCESS_2_NONE);
        return result;
    }

    inline static VkAccessFlags2 GetAccessFlags2(BufferUsage usage, TL::Flags<Access> access)
    {
        VkAccessFlags2 result = VK_ACCESS_2_NONE;
        switch (usage)
        {
        case BufferUsage::Vertex:
            result |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
            TL_ASSERT((access & Access::Write) == Access::None, "BufferUsage::Vertex can't have write access");
            break;
        case BufferUsage::Index:
            result |= VK_ACCESS_2_INDEX_READ_BIT;
            TL_ASSERT((access & Access::Write) == Access::None, "BufferUsage::Index can't have write access");
            break;
        case BufferUsage::Uniform:
            if (access & Access::Read) result |= VK_ACCESS_2_UNIFORM_READ_BIT;
            TL_ASSERT((access & Access::Write) == Access::None, "BufferUsage::Uniform can't have write access");
            break;
        case BufferUsage::Storage:
            if (access & Access::Read) result |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            break;
        case BufferUsage::CopySrc:
        case BufferUsage::CopyDst:
            if (access & Access::Read) result |= VK_ACCESS_2_TRANSFER_READ_BIT;
            if (access & Access::Write) result |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        default: break;
        };
        TL_ASSERT(result != VK_ACCESS_2_NONE);
        return result;
    }

    inline static VkImageLayout GetImageLayout(ImageUsage usage, TL::Flags<Access> access, TL::Flags<ImageAspect> aspect)
    {
        switch (usage)
        {
        case ImageUsage::ShaderResource:
            {
                bool isReadOnly = access == Access::Read;

                if (aspect & ImageAspect::Color)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
                }
                else if (aspect & ImageAspect::DepthStencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else if (aspect & ImageAspect::Depth)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL
                                      : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                }
                else if (aspect & ImageAspect::Stencil)
                {
                    return isReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
                                      : VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                }

                TL_UNREACHABLE();
                return VK_IMAGE_LAYOUT_GENERAL;
            }
        case ImageUsage::Color:             return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageUsage::Depth:             return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        case ImageUsage::Stencil:           return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::DepthStencil:      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::CopySrc:           return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageUsage::CopyDst:           return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageUsage::_SwapchainPresent: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default:                            return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    IRenderGraph::IRenderGraph()  = default;
    IRenderGraph::~IRenderGraph() = default;

    ResultCode IRenderGraph::Init([[maybe_unused]] IDevice* device)
    {
        this->m_device = device;
        return ResultCode::Success;
    }

    void IRenderGraph::Shutdown()
    {
    }

    void IRenderGraph::OnGraphExecutionBegin()
    {
        ZoneScoped;
    }

    void IRenderGraph::OnGraphExecutionEnd()
    {
        [[maybe_unused]] auto res = m_graphImportedSwapchainsLookup.begin()->first->Present();
        // auto res                   = swapchain->Present();
    }

    void IRenderGraph::ExecutePassGroup(const RenderGraphExecuteGroup& passGroup, QueueType queueType)
    {
        auto device = (IDevice*)m_device;

        auto commandList = (ICommandList*)m_device->CreateCommandList({.queueType = queueType});
        commandList->Begin();
        for (auto pass : passGroup.passList)
        {
            commandList->DebugMarkerPush(pass->GetName(), QueueTypeToColor(queueType));
            EmitBarriers(*commandList, *pass, BarrierSlot::Prilogue);
            commandList->BeginPass(*pass);
            ExecutePassCallback(*pass, *commandList);
            commandList->EndPass();
            EmitBarriers(*commandList, *pass, BarrierSlot::Epilogue);
            commandList->DebugMarkerPop();

            // maybe add execution barrier if next pass depends on any pass prior passes
        }
        commandList->End();

        TL::Vector<VkSemaphoreSubmitInfo, TL::IAllocator> waitSemaphores(m_tempAllocator);
        if (passGroup.swapchainToAcquire.swapchain)
        {
            VkSemaphoreSubmitInfo acquireSemaphore = {
                .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext       = nullptr,
                .semaphore   = ((ISwapchain*)passGroup.swapchainToAcquire.swapchain)->GetImageAcquiredSemaphore(),
                .value       = 0,
                .stageMask   = ConvertPipelineStageFlags(passGroup.swapchainToAcquire.stages),
                .deviceIndex = 0,
            };
            waitSemaphores.push_back(acquireSemaphore);
        }

        // Handle async queue dependencies
        for (const auto& dependency : passGroup.asyncQueuesDependencies)
        {
            if (dependency.localQueuePassIndex != UINT16_MAX) // Valid dependency
            {
                TL_UNREACHABLE(); // untested
                VkSemaphoreSubmitInfo asyncSemaphore = {
                    .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    .pNext       = nullptr,
                    .semaphore   = device->GetDeviceQueue(dependency.queueType).GetTimelineSemaphoreHandle(),
                    .value       = device->GetDeviceQueue(dependency.queueType).AdvanceTimelinePendingValue(dependency.localQueuePassIndex),
                    .stageMask   = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                    .deviceIndex = 0,
                };
                waitSemaphores.push_back(asyncSemaphore);
            }
        }

        // Handle swapchain release semaphore
        TL::Vector<VkSemaphoreSubmitInfo, TL::IAllocator> signalSemaphores(m_tempAllocator);
        if (passGroup.swapchainToRelease.swapchain)
        {
            VkSemaphoreSubmitInfo releaseSemaphore = {
                .sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .pNext       = nullptr,
                .semaphore   = ((ISwapchain*)passGroup.swapchainToRelease.swapchain)->GetImagePresentSemaphore(),
                .value       = 0,
                .stageMask   = ConvertPipelineStageFlags(passGroup.swapchainToRelease.stages),
                .deviceIndex = 0,
            };
            signalSemaphores.push_back(releaseSemaphore);
        }

        TL::Flags<PipelineStage> signalStages;
        QueueSubmitInfo          queueSubmitInfo{
                     .waitSemaphores   = waitSemaphores,
                     .commandLists     = {commandList},
                     .signalSemaphores = signalSemaphores,
                     .signalStage      = signalStages,
        };
        device->m_queue[(uint32_t)queueType]->Submit(queueSubmitInfo);
    }

    VkImageSubresourceRange IRenderGraph::GetAccessedSubresourceRange(const RenderGraphResourceTransition& accessedResource)
    {
        TL_ASSERT(accessedResource.resource->GetType() == RenderGraphResource::Type::Image);
        auto resource = (RenderGraphImage*)accessedResource.resource;

        return {
            .aspectMask     = ConvertImageAspect(GetFormatAspects(resource->GetFormat())),
            .baseMipLevel   = 0,
            .levelCount     = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount     = VK_REMAINING_ARRAY_LAYERS,
        };
    }

    BarrierStage IRenderGraph::GetBarrierStage(const RenderGraphResourceTransition* accessedResource)
    {
        if (accessedResource == nullptr) return {};

        switch (accessedResource->resource->GetType())
        {
        case RenderGraphResource::Type::Image:
            {
                auto usage  = accessedResource->asImage.usage;
                auto stage  = accessedResource->asImage.stage;
                auto access = accessedResource->asImage.access;
                return {
                    .stageMask        = ConvertPipelineStageFlags(stage),
                    .accessMask       = GetAccessFlags2(usage, access),
                    .layout           = GetImageLayout(usage, access, ImageAspect::Color),
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        case RenderGraphResource::Type::Buffer:
            {
                auto usage  = accessedResource->asBuffer.usage;
                auto stage  = accessedResource->asBuffer.stage;
                auto access = accessedResource->asBuffer.access;
                return {
                    .stageMask        = ConvertPipelineStageFlags(stage),
                    .accessMask       = GetAccessFlags2(usage, access),
                    .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        default:
            TL_UNREACHABLE();
            break;
        }
    }

    void IRenderGraph::EmitBarriers(ICommandList& commandList, Pass& pass, BarrierSlot slot)
    {
        ZoneScoped;

        auto device = (IDevice*)m_device;

        constexpr size_t MaxImageBarriers  = 32;
        constexpr size_t MaxBufferBarriers = 32;

        size_t                 imageBarrierCount = 0;
        VkImageMemoryBarrier2  imageBarriers[MaxImageBarriers];
        size_t                 bufferBarrierCount = 0;
        VkBufferMemoryBarrier2 bufferBarriers[MaxBufferBarriers];

        for (const auto& resourceTransition : pass.GetRenderGraphResourceTransitions())
        {
            auto srcResourceAccess = slot == BarrierSlot::Prilogue ? resourceTransition->prev : resourceTransition;
            auto dstResourceAccess = slot == BarrierSlot::Prilogue ? resourceTransition : resourceTransition->next;

            auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(srcResourceAccess);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQfi] = GetBarrierStage(dstResourceAccess);

            bool shouldTransferResourceQueueOwnership = srcQfi == dstQfi; /// @todo: && resource sharing is execlusive!

            if (slot != BarrierSlot::Prilogue && resourceTransition->pass != nullptr)
            {
                continue;
            }

            if (resourceTransition->resource->GetType() == RenderGraphResource::Type::Image)
            {
                if (dstLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    continue;
                }

                TL_ASSERT(imageBarrierCount < MaxImageBarriers, "Exceeded MaxImageBarriers");
                auto imageTransition = (RenderGraphImage*)resourceTransition->resource;
                auto image           = device->m_imageOwner.Get(imageTransition->GetImage());

                imageBarriers[imageBarrierCount++] = {
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext               = nullptr,
                    .srcStageMask        = srcStageMask,
                    .srcAccessMask       = srcAccessMask,
                    .dstStageMask        = dstStageMask,
                    .dstAccessMask       = dstAccessMask,
                    .oldLayout           = srcLayout,
                    .newLayout           = dstLayout,
                    .srcQueueFamilyIndex = shouldTransferResourceQueueOwnership ? VK_QUEUE_FAMILY_IGNORED : srcQfi,
                    .dstQueueFamilyIndex = shouldTransferResourceQueueOwnership ? VK_QUEUE_FAMILY_IGNORED : dstQfi,
                    .image               = image->handle,
                    .subresourceRange    = GetAccessedSubresourceRange(*resourceTransition),
                };
            }
            else
            {
                if (slot != BarrierSlot::Prilogue)
                {
                    continue;
                }

                TL_ASSERT(bufferBarrierCount < MaxBufferBarriers, "Exceeded MaxBufferBarriers");

                auto bufferTransition = (RenderGraphBuffer*)resourceTransition->resource;
                auto buffer           = device->m_bufferOwner.Get(bufferTransition->GetBuffer());

                bufferBarriers[bufferBarrierCount++] = {
                    .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                    .pNext               = nullptr,
                    .srcStageMask        = srcStageMask,
                    .srcAccessMask       = srcAccessMask,
                    .dstStageMask        = dstStageMask,
                    .dstAccessMask       = dstAccessMask,
                    .srcQueueFamilyIndex = shouldTransferResourceQueueOwnership ? VK_QUEUE_FAMILY_IGNORED : srcQfi,
                    .dstQueueFamilyIndex = shouldTransferResourceQueueOwnership ? VK_QUEUE_FAMILY_IGNORED : dstQfi,
                    .buffer              = buffer->handle,
                    .offset              = resourceTransition->asBuffer.subregion.offset,
                    .size                = resourceTransition->asBuffer.subregion.size,
                };
            }
        }

        commandList.AddPipelineBarriers({
            .imageBarriers  = {imageBarriers, imageBarrierCount},
            .bufferBarriers = {bufferBarriers, bufferBarrierCount},
        });
    }

} // namespace RHI::Vulkan