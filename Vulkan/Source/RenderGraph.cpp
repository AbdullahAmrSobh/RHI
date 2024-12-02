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
    }

    inline static uint32_t QueueTypeToQueueFamilyIndex(IDevice& device, QueueType queueType)
    {
        return device.m_queue[(uint32_t)queueType]->GetFamilyIndex();
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

    inline static VkAccessFlags2 GetAccessFlags2(ImageUsage usage, TL::Flags<Access> access)
    {
        switch (usage)
        {
        case ImageUsage::ShaderResource:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT;
                }
                else if (access == Access::Write)
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
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
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
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT;
                }
                else if (access == Access::Write)
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

    inline static VkAccessFlags2 GetAccessFlags2(BufferUsage usage, TL::Flags<Access> access)
    {
        switch (usage)
        {
        case BufferUsage::None: return {};
        case BufferUsage::Uniform:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case BufferUsage::Storage:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
                }
                else if (access == Access::Write)
                {
                    return VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
                }
                else
                {
                    TL_UNREACHABLE();
                }
            }
            break;
        case BufferUsage::CopySrc:
        case BufferUsage::CopyDst:
            {
                if (access == Access::ReadWrite)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
                }
                else if (access == Access::Read)
                {
                    return VK_ACCESS_2_TRANSFER_READ_BIT;
                }
                else if (access == Access::Write)
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
        case ImageUsage::CopySrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageUsage::CopyDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        default:                  return VK_IMAGE_LAYOUT_GENERAL;
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

        auto device = (IDevice*)m_device;

        m_asyncQueuesTimelineValues[(uint32_t)QueueType::Graphics] = device->m_queue[(uint32_t)QueueType::Graphics]->GetTimelineSemaphorePendingValue();
        m_asyncQueuesTimelineValues[(uint32_t)QueueType::Compute]  = device->m_queue[(uint32_t)QueueType::Compute]->GetTimelineSemaphorePendingValue();
        m_asyncQueuesTimelineValues[(uint32_t)QueueType::Transfer] = device->m_queue[(uint32_t)QueueType::Transfer]->GetTimelineSemaphorePendingValue();
    }

    void IRenderGraph::OnGraphExecutionEnd()
    {
        auto device                = (IDevice*)m_device;
        auto [swapchain, resource] = *m_graphImportedSwapchainsLookup.begin();

        auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(resource->GetLastAccess());

        auto commandList = (ICommandList*)m_device->CreateCommandList({.queueType = QueueType::Graphics});
        commandList->Begin();
        commandList->AddPipelineBarriers({
            .imageBarriers = {
                {
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext               = nullptr,
                    .srcStageMask        = srcStageMask,
                    .srcAccessMask       = srcAccessMask,
                    .dstStageMask        = 0,
                    .dstAccessMask       = 0,
                    .oldLayout           = srcLayout,
                    .newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image               = device->m_imageOwner.Get(swapchain->GetImage())->handle,
                    .subresourceRange    = {
                           .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                           .baseMipLevel   = 0,
                           .levelCount     = VK_REMAINING_MIP_LEVELS,
                           .baseArrayLayer = 0,
                           .layerCount     = VK_REMAINING_ARRAY_LAYERS,
                    },

                },
            },
        });
        commandList->End();

        device->m_queue[0]->Submit({.commandLists = commandList});

        auto res = swapchain->Present();
    }

    void IRenderGraph::ExecutePassGroup(const PassGroup& passGroup, QueueType queueType)
    {
        auto device      = (IDevice*)m_device;
        /// @todo: dispatch worker thread to execute each pass within the given passGroup.
        /// when the threading model is clear
        auto commandList = (ICommandList*)m_device->CreateCommandList({.queueType = queueType});
        commandList->Begin();
        for (auto pass : passGroup.passList)
        {
            commandList->DebugMarkerPush(pass->GetName(), QueueTypeToColor(queueType));
            EmitBarriers(*commandList, *pass, BarrierSlot_Prilogue);
            commandList->BeginPass(*pass);
            ExecutePassCallback(*pass, *commandList);
            /// @todo: handle resolve here
            commandList->EndPass();
            // EmitBarriers(*commandList, *pass, BarrierSlot_Epilogue);
            commandList->DebugMarkerPop();
        }
        commandList->End();

        auto graphicsQueueIndex = (uint32_t)QueueType::Graphics;
        // auto computeQueueIndex  = (uint32_t)QueueType::Compute;
        // auto transferQueueIndex = (uint32_t)QueueType::Transfer;

        auto& graphicsQueue = device->m_queue[graphicsQueueIndex];
        // auto& computeQueue  = device->m_queue[computeQueueIndex];
        // auto& transferQueue = device->m_queue[transferQueueIndex];

        uint64_t pendingQueuesTimelineValues[AsyncQueuesCount] = {};
        // pendingQueuesTimelineValues[graphicsQueueIndex]        = m_asyncQueuesTimelineValues[graphicsQueueIndex] + passGroup.ssis[graphicsQueueIndex];
        // pendingQueuesTimelineValues[computeQueueIndex]         = m_asyncQueuesTimelineValues[computeQueueIndex] + passGroup.ssis[computeQueueIndex];
        // pendingQueuesTimelineValues[transferQueueIndex]        = m_asyncQueuesTimelineValues[transferQueueIndex] + passGroup.ssis[transferQueueIndex];

        VkSemaphoreSubmitInfo binaryWaitSemaphore{};
        if (auto swapchainAccess = passGroup.swapchainAcquire)
        {
            // auto [swapchain, stage] = swapchainAccess;
            binaryWaitSemaphore = {
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = ((ISwapchain*)passGroup.swapchainAcquire)->GetImageAcquiredSemaphore(),
                // .stageMask = ConvertPipelineStageFlags(stage),
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            };
        }

        VkSemaphoreSubmitInfo binarySignalSemaphore{};
        if (auto swapchainAccess = passGroup.swapchainRelease)
        {
            // auto [swapchain, stage] = swapchainAccess->asSwapchain;
            binaryWaitSemaphore = {
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = ((ISwapchain*)passGroup.swapchainRelease)->GetImagePresentSemaphore(),
                // .stageMask = ConvertPipelineStageFlags(stage),
                .stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
            };
        }

        QueueSubmitInfo queueSubmitInfo{
            /// @fixme: Should check if async queues are present to avoid passing dublicate semaphore handles (with different sync values)
            .timelineWaitSemaphores =
                {
                    // Waits for graphics queue
                    {
                        .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                        .semaphore = graphicsQueue->GetTimelineSemaphoreHandle(),
                        .value     = device->m_queue[0]->GetTimelineSemaphorePendingValue(),
                    },
                    // // Waits for async compute queue
                    // {
                    //     .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    //     .semaphore = computeQueue->GetTimelineSemaphoreHandle(),
                    //     .value     = pendingQueuesTimelineValues[computeQueueIndex],
                    // },
                    // // Waits for async transfer queue
                    // {
                    //     .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                    //     .semaphore = transferQueue->GetTimelineSemaphoreHandle(),
                    //     .value     = pendingQueuesTimelineValues[transferQueueIndex],
                    // },
                },
            .binaryWaitSemaphore   = binaryWaitSemaphore,
            .binarySignalSemaphore = binarySignalSemaphore,
            .commandLists          = commandList,
        };
        pendingQueuesTimelineValues[(uint32_t)queueType] = device->m_queue[(uint32_t)queueType]->Submit(queueSubmitInfo);
    }

    VkImageSubresourceRange IRenderGraph::GetAccessedSubresourceRange(const PassAccessedResource& accessedResource)
    {
        return {
            .aspectMask     = ConvertImageAspect(GetFormatAspects(accessedResource.asImage.image->GetFormat())),
            .baseMipLevel   = 0,
            .levelCount     = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount     = VK_REMAINING_ARRAY_LAYERS,
        };
    }

    BarrierStage IRenderGraph::GetBarrierStage(const PassAccessedResource* accessedResource)
    {
        if (accessedResource == nullptr) return {};

        switch (accessedResource->type)
        {
        case RenderGraphResourceAccessType::None:
            {
                TL_UNREACHABLE();
                return {};
            }
            break;
        case RenderGraphResourceAccessType::Image:
            {
                auto usage  = accessedResource->asImage.usage;
                auto stage  = accessedResource->asImage.stage;
                auto access = accessedResource->asImage.access;
                return {
                    .stageMask        = ConvertPipelineStageFlags(stage),
                    .accessMask       = GetAccessFlags2(usage, access),
                    // TODO: might have stencil!!
                    .layout           = GetImageLayout(usage, access, ImageAspect::Color),
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        case RenderGraphResourceAccessType::Buffer:
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
        case RenderGraphResourceAccessType::RenderTarget:
            {
                auto format = GetFormatInfo(accessedResource->asImage.image->GetFormat());

                VkPipelineStageFlags2 stage  = VK_PIPELINE_STAGE_2_NONE;
                VkAccessFlags2        access = VK_ACCESS_2_NONE;
                VkImageLayout         layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                if ((format.hasDepth || format.hasStencil) == false)
                {
                    if (accessedResource->asRenderTarget.loadOperation == LoadOperation::Load)
                        access |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
                    if (accessedResource->asRenderTarget.storeOperation == StoreOperation::Store)
                        access |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

                    stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                }
                else
                {
                    if (format.hasDepth && format.hasStencil)
                    {
                        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                        if (accessedResource->asRenderTarget.loadOperation == LoadOperation::Load)
                            access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                        if (accessedResource->asRenderTarget.storeOperation == StoreOperation::Store)
                            access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    }
                    else if (format.hasDepth)
                    {
                        layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                        if (accessedResource->asRenderTarget.loadOperation == LoadOperation::Load)
                            access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                        if (accessedResource->asRenderTarget.storeOperation == StoreOperation::Store)
                            access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    }
                    else if (format.hasStencil)
                    {
                        layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                        if (accessedResource->asRenderTarget.stencilLoadOperation == LoadOperation::Load)
                            access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                        if (accessedResource->asRenderTarget.stencilStoreOperation == StoreOperation::Store)
                            access |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    }

                    stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                }

                return {
                    .stageMask        = stage,
                    .accessMask       = access,
                    .layout           = layout,
                    .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                };
            }
            break;
        case RenderGraphResourceAccessType::Resolve:
            {
                TL_UNREACHABLE();
            }
            break;
        case RenderGraphResourceAccessType::SwapchainPresent:
            {
                return {
                    .stageMask        = VK_PIPELINE_STAGE_NONE,
                    .accessMask       = VK_ACCESS_NONE,
                    .layout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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

        for (const auto& accessedResource : pass.GetAccessedResources())
        {
            auto srcResourceAccess = slot == BarrierSlot_Prilogue ? accessedResource->prev : accessedResource;
            auto dstResourceAccess = slot == BarrierSlot_Prilogue ? accessedResource : accessedResource->next;

            auto [srcStageMask, srcAccessMask, srcLayout, srcQfi] = GetBarrierStage(srcResourceAccess);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQfi] = GetBarrierStage(dstResourceAccess);

            bool shouldTransferResourceQueueOwnership = srcQfi == dstQfi; /// @todo: && resource sharing is execlusive!

            if (slot != BarrierSlot_Prilogue && accessedResource->pass != nullptr)
            {
                continue;
            }

            if (accessedResource->type != RenderGraphResourceAccessType::Buffer)
            {
                if (dstLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    continue;
                }

                TL_ASSERT(imageBarrierCount < MaxImageBarriers, "Exceeded MaxImageBarriers");

                auto imageAccess = accessedResource->asImage;
                auto image       = device->m_imageOwner.Get(imageAccess.image->GetImage());

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
                    .subresourceRange    = GetAccessedSubresourceRange(*accessedResource),
                };
            }
            else
            {
                if (slot != BarrierSlot_Prilogue)
                {
                    continue;
                }

                TL_ASSERT(bufferBarrierCount < MaxBufferBarriers, "Exceeded MaxBufferBarriers");

                auto bufferAccess = accessedResource->asBuffer;
                auto buffer       = device->m_bufferOwner.Get(bufferAccess.buffer->GetBuffer());

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
                    .offset              = accessedResource->asBuffer.subregion.offset,
                    .size                = accessedResource->asBuffer.subregion.size,
                };
            }
        }

        commandList.AddPipelineBarriers({
            .imageBarriers  = {imageBarriers, imageBarrierCount},
            .bufferBarriers = {bufferBarriers, bufferBarrierCount},
        });
    }

} // namespace RHI::Vulkan