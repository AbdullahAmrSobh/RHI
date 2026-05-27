#include "CommandList.hpp"
#include "Common.hpp"
#include "Device.hpp"
#include "Resources.hpp"

#include <RHI/Format.hpp>
#include <TL/Containers/Optional.hpp>

#include <algorithm>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    inline static VkImageSubresourceLayers ConvertSubresourceLayers(const ImageCopyInfo& copyInfo, Format format)
    {
        return VkImageSubresourceLayers{
            .aspectMask     = ConvertImageAspect(copyInfo.aspect, format),
            .mipLevel       = copyInfo.mipLevel,
            .baseArrayLayer = copyInfo.arrayLayer,
            .layerCount     = 1,
        };
    }

    inline static VkResolveModeFlagBits ConvertResolveMode(ResolveMode resolveMode)
    {
        switch (resolveMode)
        {
        case ResolveMode::None: return VK_RESOLVE_MODE_NONE;
        case ResolveMode::Min:  return VK_RESOLVE_MODE_MIN_BIT;
        case ResolveMode::Max:  return VK_RESOLVE_MODE_MAX_BIT;
        case ResolveMode::Avg:  return VK_RESOLVE_MODE_AVERAGE_BIT;
        }
        return {};
    }

    struct BarrierStage
    {
        VkPipelineStageFlags2 stageMask        = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2        accessMask       = VK_ACCESS_2_NONE;
        VkImageLayout         layout           = VK_IMAGE_LAYOUT_UNDEFINED;
        uint32_t              queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    };

    inline static BarrierStage ConvertBarrierState(TL_MAYBE_UNUSED const BarrierState& barrierState)
    {
        return {
            // .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            // .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            // .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
            // .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static BarrierStage ConvertBarrierState(const ImageBarrierState& barrierState)
    {
        return {
            .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            .layout           = GetImageLayout(barrierState.usage, barrierState.access, ImageAspect::All),
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static BarrierStage ConvertBarrierState(const BufferBarrierState& barrierState)
    {
        return {
            .stageMask        = ConvertPipelineStageFlags(barrierState.stage),
            .accessMask       = GetAccessFlags2(barrierState.usage, barrierState.access),
            .layout           = VK_IMAGE_LAYOUT_UNDEFINED,
            .queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        };
    }

    inline static VkStridedDeviceAddressRegionKHR convertStridedDeviceAddressRegion(const StridedDeviceAddressRegion& r)
    {
        return VkStridedDeviceAddressRegionKHR{
            .deviceAddress = (VkDeviceAddress)r.offset,
            .stride        = r.stride,
            .size          = r.size,
        };
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    /// ICommandPool
    //////////////////////////////////////////////////////////////////////////////////////////

    ResultCode ICommandPool::Init(IDevice* device, const CommandPoolCreateInfo& createInfo)
    {
        m_device      = device;
        IQueue* queue = (IQueue*)device->GetQueue(createInfo.queue);

        VkCommandPoolCreateInfo poolInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue->m_familyIndex,
        };

        VulkanResult result = vkCreateCommandPool(device->m_device, &poolInfo, nullptr, &m_commandPool);
        return result;
    }

    void ICommandPool::Shutdown(IDevice* device)
    {
        vkDestroyCommandPool(device->m_device, m_commandPool, nullptr);
    }

    void ICommandPool::Reset()
    {
        vkResetCommandPool(m_device->m_device, m_commandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        m_arena.reset();
    }

    CommandList* ICommandPool::Allocate()
    {
        VkCommandBufferAllocateInfo allocateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            nullptr,
            m_commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1,
        };
        ICommandList* commandList = TL::constructFrom<ICommandList>(&m_arena);
        commandList->m_device     = m_device;
        VulkanResult result       = vkAllocateCommandBuffers(m_device->m_device, &allocateInfo, &commandList->m_commandBuffer);
        TL_ASSERT(result.IsSuccess());
        return commandList;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList()  = default;
    ICommandList::~ICommandList() = default;

    ResultCode ICommandList::Init(IDevice* device, CommandPool* pool, const CommandListCreateInfo& createInfo)
    {
        // m_device        = device;
        // m_commandBuffer = pool->AllocateCommandBuffer(m_device);
        return ResultCode::Success;
    }

    void ICommandList::Shutdown()
    {
        // m_device->m_commandsAllocator->ReleaseCommandBuffers(m_commandBuffer);
    }

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        if (bindGroups.empty())
            return;

        TL::Vector<VkDescriptorSet> descriptorSets{m_device->m_arena};
        TL::Vector<uint32_t>        dynamicOffsets{m_device->m_arena};

        for (const auto& bindingInfo : bindGroups)
        {
            auto bindGroup = (IBindGroup*)bindingInfo.bindGroup;
            descriptorSets.push_back(bindGroup->descriptorSet);
            for (uint32_t offset : bindingInfo.dynamicOffsets)
            {
                dynamicOffsets.push_back(offset);
            }
        }

        VkBindDescriptorSetsInfo bindInfo{
            .sType              = VK_STRUCTURE_TYPE_BIND_DESCRIPTOR_SETS_INFO,
            .pNext              = nullptr,
            .stageFlags         = VK_SHADER_STAGE_ALL,
            .layout             = pipelineLayout,
            .firstSet           = 0,
            .descriptorSetCount = (uint32_t)descriptorSets.size(),
            .pDescriptorSets    = descriptorSets.data(),
            .dynamicOffsetCount = (uint32_t)dynamicOffsets.size(),
            .pDynamicOffsets    = dynamicOffsets.data(),
        };
        vkCmdBindDescriptorSets2(m_commandBuffer, &bindInfo);
    }

    void ICommandList::Begin()
    {
        ZoneScoped;

        VkCommandBufferBeginInfo beginInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .pInheritanceInfo = nullptr,
        };
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void ICommandList::End()
    {
        ZoneScoped;

        vkEndCommandBuffer(m_commandBuffer);
    }

    void ICommandList::PushDebugMarker(TL_MAYBE_UNUSED const char* name, TL_MAYBE_UNUSED uint32_t bgra)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = vkCmdBeginDebugUtilsLabelEXT)
        {
            uint32_t             r = (bgra >> 16) & 0xFF;
            uint32_t             g = (bgra >> 8) & 0xFF;
            uint32_t             b = bgra & 0xFF;
            uint32_t             a = (bgra >> 24) & 0xFF;
            VkDebugUtilsLabelEXT info{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f},
            };
            fn(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::PopDebugMarker()
    {
        if (auto fn = vkCmdEndDebugUtilsLabelEXT)
        {
            fn(m_commandBuffer);
        }
    }

    void ICommandList::InsertDebugMarker(TL_MAYBE_UNUSED const char* name, TL_MAYBE_UNUSED uint32_t bgra)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto fn = vkCmdInsertDebugUtilsLabelEXT)
        {
            uint32_t             r = (bgra >> 16) & 0xFF;
            uint32_t             g = (bgra >> 8) & 0xFF;
            uint32_t             b = bgra & 0xFF;
            uint32_t             a = (bgra >> 24) & 0xFF;
            VkDebugUtilsLabelEXT info{
                .sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
                .pNext      = nullptr,
                .pLabelName = name,
                .color      = {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f},
            };
            fn(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::AddPipelineBarrier(TL::Span<const BarrierInfo> barriers, TL::Span<const ImageBarrierInfo> imageBarriers, TL::Span<const BufferBarrierInfo> bufferBarriers)
    {
        ZoneScoped;

        if (barriers.empty() && imageBarriers.empty() && bufferBarriers.empty())
            return;

        TL::Vector<VkMemoryBarrier2>       vmemoryBarriers{m_device->m_arena};
        TL::Vector<VkBufferMemoryBarrier2> vbufferBarriers{m_device->m_arena};
        TL::Vector<VkImageMemoryBarrier2>  vimageBarriers{m_device->m_arena};
        vmemoryBarriers.reserve(barriers.size());
        vbufferBarriers.reserve(imageBarriers.size());
        vimageBarriers.reserve(bufferBarriers.size());

        for (auto barrier : barriers)
        {
            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(barrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(barrier.dstState);

            vmemoryBarriers.push_back({
                .sType         = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
                .pNext         = nullptr,
                .srcStageMask  = srcStageMask,
                .srcAccessMask = srcAccessMask,
                .dstStageMask  = dstStageMask,
                .dstAccessMask = dstAccessMask,
            });
        }

        for (auto imageBarrier : imageBarriers)
        {
            auto image = (IImage*)(imageBarrier.image);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(imageBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(imageBarrier.dstState);

            vimageBarriers.push_back({
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = dstStageMask,
                .dstAccessMask       = dstAccessMask,
                .oldLayout           = srcLayout,
                .newLayout           = dstLayout,
                .srcQueueFamilyIndex = srcQueueFamilyIndex,
                .dstQueueFamilyIndex = dstQueueFamilyIndex,
                .image               = image->handle,
                .subresourceRange    = ConvertSubresourceRange(image->subresources, image->format),
            });
        }

        for (auto bufferBarrier : bufferBarriers)
        {
            auto buffer = (IBuffer*)(bufferBarrier.buffer);

            auto [srcStageMask, srcAccessMask, srcLayout, srcQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.srcState);
            auto [dstStageMask, dstAccessMask, dstLayout, dstQueueFamilyIndex] = ConvertBarrierState(bufferBarrier.dstState);

            vbufferBarriers.push_back({
                .sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext               = nullptr,
                .srcStageMask        = srcStageMask,
                .srcAccessMask       = srcAccessMask,
                .dstStageMask        = dstStageMask,
                .dstAccessMask       = dstAccessMask,
                .srcQueueFamilyIndex = srcQueueFamilyIndex,
                .dstQueueFamilyIndex = dstQueueFamilyIndex,
                .buffer              = buffer->handle,
                .offset              = bufferBarrier.subregion.offset,
                .size                = VK_WHOLE_SIZE, // bufferBarrier.subregion.size,
            });
        }

        VkDependencyInfo dependencyInfo =
            {
                .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext                    = nullptr,
                .dependencyFlags          = 0,
                .memoryBarrierCount       = uint32_t(vmemoryBarriers.size()),
                .pMemoryBarriers          = vmemoryBarriers.data(),
                .bufferMemoryBarrierCount = uint32_t(vbufferBarriers.size()),
                .pBufferMemoryBarriers    = vbufferBarriers.data(),
                .imageMemoryBarrierCount  = uint32_t(vimageBarriers.size()),
                .pImageMemoryBarriers     = vimageBarriers.data(),
            };
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

    void ICommandList::BeginRenderPass(const RenderPassBeginInfo& beginInfo)
    {
        ZoneScoped;

        TL::Vector<VkRenderingAttachmentInfo>   colorAttachments{m_device->m_arena};
        TL::Optional<VkRenderingAttachmentInfo> depthAttachment{};
        TL::Optional<VkRenderingAttachmentInfo> stencilAttachment{};

        for (const auto& colorAttachment : beginInfo.colorAttachments)
        {
            auto colorImage  = (IImage*)(colorAttachment.view);
            auto resolveView = colorAttachment.resolveView ? (IImage*)(colorAttachment.resolveView) : nullptr;
            colorAttachments.push_back({
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext              = nullptr,
                .imageView          = colorImage->viewHandle,
                .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = ConvertResolveMode(colorAttachment.resolveMode),
                .resolveImageView   = resolveView ? resolveView->viewHandle : VK_NULL_HANDLE,
                .resolveImageLayout = resolveView ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp             = ConvertLoadOp(colorAttachment.loadOp),
                .storeOp            = ConvertStoreOp(colorAttachment.storeOp),
                .clearValue         = {colorAttachment.clearValue.f32.r, colorAttachment.clearValue.f32.g, colorAttachment.clearValue.f32.b, colorAttachment.clearValue.f32.a},
            });
        }

        if (beginInfo.depthStencilAttachment.view)
        {
            auto image = (IImage*)(beginInfo.depthStencilAttachment.view);

            if (image->subresources.imageAspects & ImageAspect::Depth)
            {
                depthAttachment = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = image->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .resolveMode        = VK_RESOLVE_MODE_NONE,
                    .resolveImageView   = VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp             = ConvertLoadOp(beginInfo.depthStencilAttachment.depthLoadOp),
                    .storeOp            = ConvertStoreOp(beginInfo.depthStencilAttachment.depthStoreOp),
                    .clearValue         = {.depthStencil = {.depth = beginInfo.depthStencilAttachment.clearValue.depthValue, .stencil = beginInfo.depthStencilAttachment.clearValue.stencilValue}},
                };
            }

            if (image->subresources.imageAspects & ImageAspect::Stencil)
            {
                stencilAttachment = VkRenderingAttachmentInfo{
                    .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext              = nullptr,
                    .imageView          = image->viewHandle,
                    .imageLayout        = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                    .resolveMode        = VK_RESOLVE_MODE_NONE,
                    .resolveImageView   = VK_NULL_HANDLE,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp             = ConvertLoadOp(beginInfo.depthStencilAttachment.stencilLoadOp),
                    .storeOp            = ConvertStoreOp(beginInfo.depthStencilAttachment.stencilStoreOp),
                    .clearValue         = {.depthStencil = {.depth = beginInfo.depthStencilAttachment.clearValue.depthValue, .stencil = beginInfo.depthStencilAttachment.clearValue.stencilValue}},
                };
            }
            if ((image->subresources.imageAspects & ImageAspect::DepthStencil) == ImageAspect::DepthStencil)
            {
                depthAttachment->imageLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                stencilAttachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
        }

        auto [offsetX, offsetY] = beginInfo.offset;
        auto [width, height]    = beginInfo.size;
        VkRenderingInfo renderingInfo{
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext                = nullptr,
            .flags                = 0,
            .renderArea           = {.offset = {offsetX, offsetY}, .extent = {width, height}},
            .layerCount           = 1,
            .viewMask             = 0,
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
            .pColorAttachments    = colorAttachments.data(),
            .pDepthAttachment     = depthAttachment.has_value() ? &depthAttachment.value() : nullptr,
            .pStencilAttachment   = stencilAttachment.has_value() ? &stencilAttachment.value() : nullptr,
        };
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void ICommandList::EndRenderPass()
    {
        ZoneScoped;
        vkCmdEndRendering(m_commandBuffer);
    }

    void ICommandList::BeginComputePass(TL_MAYBE_UNUSED const ComputePassBeginInfo& beginInfo)
    {
        // No-Op
    }

    void ICommandList::EndComputePass()
    {
        // No-Op
    }


    void ICommandList::BeginConditionalCommands(const BufferBindingInfo& conditionBuffer, bool inverted)
    {
        ZoneScoped;

        auto buffer = (IBuffer*)(conditionBuffer.buffer);

        VkConditionalRenderingBeginInfoEXT beginInfo{
            .sType  = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT,
            .pNext  = nullptr,
            .buffer = buffer->handle,
            .offset = conditionBuffer.offset,
            .flags  = inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u,
        };
        vkCmdBeginConditionalRenderingEXT(m_commandBuffer, &beginInfo);
    }

    void ICommandList::EndConditionalCommands()
    {
        ZoneScoped;

        vkCmdEndConditionalRenderingEXT(m_commandBuffer);
    }

    void ICommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        ZoneScoped;

        TL::Vector<VkCommandBuffer> commandBuffers{m_device->m_arena};
        commandBuffers.reserve(commandLists.size());

        for (const auto* commandList : commandLists)
        {
            auto vkCmdList = static_cast<const ICommandList*>(commandList);
            commandBuffers.push_back(vkCmdList->m_commandBuffer);
        }

        vkCmdExecuteCommands(m_commandBuffer, commandBuffers.size(), commandBuffers.data());
    }

    void ICommandList::BindPipelineLayout(BindPoint bindPoint, const PipelineLayout* pipelineLayout)
    {
        ZoneScoped;

        m_pipelineLayout    = (PipelineLayout*)pipelineLayout;
        m_pipelineBindPoint = bindPoint == BindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    void ICommandList::SetPushConstants(BindPoint bindPoint, uint32_t offset, TL::Block content)
    {
        ZoneScoped;

        IPipelineLayout*    pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        VkPipelineBindPoint vkBindPoint    = bindPoint == BindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdPushConstants(m_commandBuffer, pipelineLayout->handle, pipelineLayout->pushConstantStages, offset, (uint32_t)content.size, content.ptr);
    }

    void ICommandList::PushBindGroup(BindPoint bindPoint, uint32_t firstGroup, TL::Span<const BindGroupUpdateInfo> updateInfos)
    {
        ZoneScoped;

        IPipelineLayout*    pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        IBindGroupLayout*   groupLayout    = pipelineLayout->bindGroupLayouts[firstGroup];
        DescriptorSetWriter writer{m_device, VK_NULL_HANDLE, groupLayout, m_device->m_arena};
        for (const auto& updateInfo : updateInfos)
        {
            for (auto [dstBindings, dstArrayelements, buffers] : updateInfo.buffers)
            {
                writer.BindBuffers(dstBindings, dstArrayelements, buffers);
            }

            for (auto [dstBindings, dstArrayelements, images] : updateInfo.images)
            {
                writer.BindImages(dstBindings, dstArrayelements, images);
            }

            for (auto [dstBindings, dstArrayelements, samplers] : updateInfo.samplers)
            {
                writer.BindSamplers(dstBindings, dstArrayelements, samplers);
            }
        }
        VkPushDescriptorSetInfo pushDescriptorSetInfo{
            .sType                = VK_STRUCTURE_TYPE_PUSH_DESCRIPTOR_SET_INFO_KHR,
            .pNext                = nullptr,
            .stageFlags           = VK_SHADER_STAGE_ALL,
            .layout               = pipelineLayout->handle,
            .set                  = firstGroup,
            .descriptorWriteCount = (uint32_t)writer.GetWrites().size(),
            .pDescriptorWrites    = writer.GetWrites().data(),
        };
        vkCmdPushDescriptorSet2KHR(m_commandBuffer, &pushDescriptorSetInfo);
    }

    void ICommandList::SetBindGroups(BindPoint bindPoint, TL::Span<const BindGroupBindingInfo> bindGroups)
    {
        ZoneScoped;

        IPipelineLayout*    pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        VkPipelineBindPoint vkBindPoint    = bindPoint == BindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

        BindShaderBindGroups(vkBindPoint, pipelineLayout->handle, bindGroups);
    }

    void ICommandList::BindGraphicsPipeline(const GraphicsPipeline* pipelineState)
    {
        ZoneScoped;

        if (pipelineState == nullptr)
        {
            m_isGraphicsPipelineBound = false;
            return;
        }

        m_isGraphicsPipelineBound       = true;
        IGraphicsPipeline* pipeline     = (IGraphicsPipeline*)(pipelineState);
        m_pipelineLayout                = pipeline->layout;
        IPipelineLayout* pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        m_pipelineBindPoint             = VK_PIPELINE_BIND_POINT_GRAPHICS;

        vkCmdBindPipeline(m_commandBuffer, m_pipelineBindPoint, pipeline->handle);
    }

    void ICommandList::BindComputePipeline(const ComputePipeline* pipelineState)
    {
        ZoneScoped;

        if (pipelineState == nullptr)
        {
            m_isComputePipelineBound = false;
            return;
        }

        m_isComputePipelineBound        = true;
        IComputePipeline* pipeline      = (IComputePipeline*)(pipelineState);
        m_pipelineLayout                = pipeline->layout;
        IPipelineLayout* pipelineLayout = (IPipelineLayout*)m_pipelineLayout;
        m_pipelineBindPoint             = VK_PIPELINE_BIND_POINT_COMPUTE;

        vkCmdBindPipeline(m_commandBuffer, m_pipelineBindPoint, pipeline->handle);
    }

    void ICommandList::BindRayTracingPipeline(const RayTracingPipeline* pipelineState)
    {
        ZoneScoped;

        if (pipelineState == nullptr)
            return;

        IRayTracingPipeline* pipeline = (IRayTracingPipeline*)(pipelineState);
        m_pipelineLayout              = pipeline->layout;
        m_pipelineBindPoint           = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;

        vkCmdBindPipeline(m_commandBuffer, m_pipelineBindPoint, pipeline->handle);
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        ZoneScoped;
        // Flip the viewport so Vulkan NDC is consitant with other APIs
        VkViewport vkViewport{
            .x        = viewport.offsetX,
            .y        = viewport.offsetY + viewport.height,
            .width    = viewport.width,
            .height   = -viewport.height,
            .minDepth = viewport.minDepth,
            .maxDepth = viewport.maxDepth,
        };
        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
        m_hasViewportSet = true;
    }

    void ICommandList::SetScissor(const Scissor& scissor)
    {
        ZoneScoped;

        VkRect2D vkScissor{
            .offset = {scissor.offsetX, scissor.offsetY},
            .extent = {scissor.width, scissor.height},
        };
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
        m_hasScissorSet = true;
    }

    void ICommandList::BindVertexBuffers(uint32_t firstBinding, TL::Span<const BufferBindingInfo> vertexBuffers)
    {
        ZoneScoped;

        constexpr size_t MaxVertexBuffers = 16;

        VkBuffer     buffers[MaxVertexBuffers];
        VkDeviceSize offsets[MaxVertexBuffers];

        size_t vertexBufferCount = vertexBuffers.size();
        TL_ASSERT(vertexBufferCount <= MaxVertexBuffers, "Vertex buffer count exceeds MaxVertexBuffers!");

        for (size_t i = 0; i < vertexBufferCount; ++i)
        {
            const auto& bindingInfo = vertexBuffers[i];
            auto        buffer      = (IBuffer*)(bindingInfo.buffer);

            buffers[i] = buffer->handle;
            offsets[i] = bindingInfo.offset;
        }

        vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, static_cast<uint32_t>(vertexBufferCount), buffers, offsets);
        m_hasVertexBuffer = true;
    }

    void ICommandList::BindIndexBuffer(const BufferBindingInfo& indexBuffer, IndexType indexType)
    {
        ZoneScoped;

        auto buffer = (IBuffer*)(indexBuffer.buffer);
        vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, indexBuffer.offset, indexType == IndexType::uint32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
        m_hasIndexBuffer = true;
    }

    void ICommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet);
        vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void ICommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet);
        vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void ICommandList::DrawMeshTasks(uint32_t x, uint32_t y, uint32_t z)
    {
        vkCmdDrawMeshTasksEXT(m_commandBuffer, x, y, z);
    }

    void ICommandList::DrawIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);
            vkCmdDrawIndirectCount(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, countBuf->handle, countBuffer.offset, maxDrawCount, stride);
        }
        else
        {
            vkCmdDrawIndirect(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, maxDrawCount, stride);
        }
    }

    void ICommandList::DrawIndexedIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t maxDrawCount, uint32_t stride)
    {
        ZoneScoped;

        TL_ASSERT(m_isGraphicsPipelineBound && m_hasViewportSet && m_hasScissorSet && m_hasVertexBuffer && m_hasIndexBuffer);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);
            vkCmdDrawIndexedIndirectCount(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, countBuf->handle, countBuffer.offset, maxDrawCount, stride);
        }
        else
        {
            vkCmdDrawIndexedIndirect(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, maxDrawCount, stride);
        }
    }

    void ICommandList::DrawMeshTasksIndirect(const BufferBindingInfo& argumentBuffer, const BufferBindingInfo& countBuffer, uint32_t drawNum, uint32_t stride)
    {
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);

        if (countBuffer.buffer != nullptr)
        {
            auto countBuf = (IBuffer*)(countBuffer.buffer);

            vkCmdDrawMeshTasksIndirectCountEXT(
                m_commandBuffer,
                cmdBuffer->handle,
                argumentBuffer.offset,
                countBuf->handle,
                countBuffer.offset,
                drawNum,
                stride);
        }
        else
        {
            vkCmdDrawMeshTasksIndirectEXT(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset, drawNum, stride);
        }
    }

    void ICommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
    {
        ZoneScoped;

        TL_ASSERT(m_isComputePipelineBound);
        vkCmdDispatch(m_commandBuffer, x, y, z);
    }

    void ICommandList::DispatchIndirect(const BufferBindingInfo& argumentBuffer)
    {
        ZoneScoped;

        TL_ASSERT(m_isComputePipelineBound);
        auto cmdBuffer = (IBuffer*)(argumentBuffer.buffer);
        vkCmdDispatchIndirect(m_commandBuffer, cmdBuffer->handle, argumentBuffer.offset);
    }

    void ICommandList::DispatchRays(const DispatchRaysInfo& dispatchRaysDesc)
    {
        VkStridedDeviceAddressRegionKHR raygen   = convertStridedDeviceAddressRegion(dispatchRaysDesc.raygenShader);
        VkStridedDeviceAddressRegionKHR miss     = convertStridedDeviceAddressRegion(dispatchRaysDesc.missShaders);
        VkStridedDeviceAddressRegionKHR hit      = convertStridedDeviceAddressRegion(dispatchRaysDesc.hitShaderGroups);
        VkStridedDeviceAddressRegionKHR callable = convertStridedDeviceAddressRegion(dispatchRaysDesc.callableShaders);
        vkCmdTraceRaysKHR(m_commandBuffer, &raygen, &miss, &hit, &callable, dispatchRaysDesc.x, dispatchRaysDesc.y, dispatchRaysDesc.z);
    }

    void ICommandList::DispatchRaysIndirect(const BufferBindingInfo& argumentBuffer)
    {
        auto            cmdBuffer             = (IBuffer*)(argumentBuffer.buffer);
        VkDeviceAddress indirectDeviceAddress = cmdBuffer->address + argumentBuffer.offset;
        vkCmdTraceRaysIndirect2KHR(m_commandBuffer, indirectDeviceAddress);
    }

    void ICommandList::CopyBuffer(const Buffer* srcBuffer, uint64_t srcOffset, const Buffer* dstBuffer, uint64_t dstOffset, uint64_t size)
    {
        ZoneScoped;

        auto src = (const IBuffer*)(srcBuffer);
        auto dst = (const IBuffer*)(dstBuffer);

        VkBufferCopy bufferCopy{
            .srcOffset = srcOffset,
            .dstOffset = dstOffset,
            .size      = size,
        };
        vkCmdCopyBuffer(m_commandBuffer, src->handle, dst->handle, 1, &bufferCopy);
    }

    void ICommandList::CopyImage(const ImageCopyInfo& srcImage, const ImageCopyInfo& dstImage, const ImageSize3D& size)
    {
        ZoneScoped;

        auto src = (const IImage*)(srcImage.image);
        auto dst = (const IImage*)(dstImage.image);

        VkImageCopy imageCopy{
            .srcSubresource = ConvertSubresourceLayers(srcImage, src->format),
            .srcOffset      = ConvertOffset3D(srcImage.offset),
            .dstSubresource = ConvertSubresourceLayers(dstImage, dst->format),
            .dstOffset      = ConvertOffset3D(dstImage.offset),
            .extent         = ConvertExtent3D(size),
        };
        vkCmdCopyImage(m_commandBuffer, src->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);
    }

    void ICommandList::CopyImageToBuffer(const ImageCopyInfo& srcImage, const ImageMemoryLayout& layout, const Buffer* dstBuffer)
    {
        ZoneScoped;

        auto image  = (const IImage*)(srcImage.image);
        auto buffer = (const IBuffer*)(dstBuffer);

        // TODO: Need to fix windows headers leaks here and use std::max/std::min

        VkExtent3D mipExtent{
            .width  = max(1u, image->size.width >> srcImage.mipLevel),
            .height = max(1u, image->size.height >> srcImage.mipLevel),
            .depth  = max(1u, image->size.depth >> srcImage.mipLevel),
        };
        VkBufferImageCopy bufferImageCopy{
            .bufferOffset      = layout.offset,
            .bufferRowLength   = 0, // 0 = tightly packed (rows = imageExtent.width texels)
            .bufferImageHeight = 0, // 0 = tightly packed (slices = imageExtent.height rows)
            .imageSubresource  = ConvertSubresourceLayers(srcImage, image->format),
            .imageOffset       = ConvertOffset3D(srcImage.offset),
            .imageExtent       = mipExtent,
        };
        vkCmdCopyImageToBuffer(m_commandBuffer, image->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer->handle, 1, &bufferImageCopy);
    }

    void ICommandList::CopyBufferToImage(const Buffer* srcBuffer, const ImageCopyInfo& dstImage, const ImageMemoryLayout& layout)
    {
        ZoneScoped;

        auto buffer = (const IBuffer*)(srcBuffer);
        auto image  = (const IImage*)(dstImage.image);

        VkExtent3D mipExtent{
            .width  = max(1u, image->size.width >> dstImage.mipLevel),
            .height = max(1u, image->size.height >> dstImage.mipLevel),
            .depth  = max(1u, image->size.depth >> dstImage.mipLevel),
        };
        VkBufferImageCopy bufferImageCopy{
            .bufferOffset      = layout.offset,
            .bufferRowLength   = 0, // 0 = tightly packed (rows = imageExtent.width texels)
            .bufferImageHeight = 0, // 0 = tightly packed (slices = imageExtent.height rows)
            .imageSubresource  = ConvertSubresourceLayers(dstImage, image->format),
            .imageOffset       = ConvertOffset3D(dstImage.offset),
            .imageExtent       = mipExtent,
        };
        vkCmdCopyBufferToImage(m_commandBuffer, buffer->handle, image->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

    void ICommandList::CopyAccelerationStructure(AccelerationStructure* dst, const AccelerationStructure* src, CopyMode copyMode)
    {
        TL_UNREACHABLE();
    }

    void ICommandList::CopyMicromap(Micromap* dst, const Micromap* src, CopyMode copyMode)
    {
        TL_UNREACHABLE();
    }

    void ICommandList::BuildTlas(TL::Span<const TlasBuildInfo> buildInfos)
    {
        ZoneScoped;

        if (buildInfos.empty())
            return;

        TL::Vector<VkAccelerationStructureGeometryKHR>              geometries{m_device->m_arena};
        TL::Vector<VkAccelerationStructureBuildGeometryInfoKHR>     geometryInfos{m_device->m_arena};
        TL::Vector<VkAccelerationStructureBuildRangeInfoKHR>        rangeInfos{m_device->m_arena};
        TL::Vector<const VkAccelerationStructureBuildRangeInfoKHR*> pRangeInfos{m_device->m_arena};

        geometries.resize(buildInfos.size());
        geometryInfos.resize(buildInfos.size());
        rangeInfos.resize(buildInfos.size());
        pRangeInfos.resize(buildInfos.size());

        for (size_t i = 0; i < buildInfos.size(); ++i)
        {
            const auto& info        = buildInfos[i];
            auto*       dstAS       = (IAccelerationStructure*)info.dst;
            auto*       srcAS       = (IAccelerationStructure*)info.src;
            auto*       instanceBuf = (const IBuffer*)info.instanceBuffer;
            auto*       scratchBuf  = (const IBuffer*)info.scratchBuffer;

            geometries[i] = VkAccelerationStructureGeometryKHR{
                .sType        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
                .pNext        = nullptr,
                .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
                .geometry     = {
                    .instances = {
                        .sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                        .pNext           = nullptr,
                        .arrayOfPointers = VK_FALSE,
                        .data            = {.deviceAddress = instanceBuf->address + info.instanceBufferOffset},
                    },
                },
                .flags = {},
            };

            geometryInfos[i] = VkAccelerationStructureBuildGeometryInfoKHR{
                .sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .pNext                    = nullptr,
                .type                     = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
                .flags                    = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                .mode                     = srcAS ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .srcAccelerationStructure = srcAS ? srcAS->handle : VK_NULL_HANDLE,
                .dstAccelerationStructure = dstAS->handle,
                .geometryCount            = 1,
                .pGeometries              = &geometries[i],
                .ppGeometries             = nullptr,
                .scratchData              = {.deviceAddress = scratchBuf->address + info.scratchBufferOffset},
            };

            rangeInfos[i] = VkAccelerationStructureBuildRangeInfoKHR{
                .primitiveCount  = info.instanceCount,
                .primitiveOffset = 0,
                .firstVertex     = 0,
                .transformOffset = 0,
            };
            pRangeInfos[i] = &rangeInfos[i];
        }

        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer, (uint32_t)geometryInfos.size(), geometryInfos.data(), pRangeInfos.data());
    }

    void ICommandList::BuildBlas(TL::Span<const BlasBuildInfo> buildInfos)
    {
        ZoneScoped;

        if (buildInfos.empty())
            return;

        TL::Vector<VkAccelerationStructureGeometryKHR>              geometries{m_device->m_arena};
        TL::Vector<VkAccelerationStructureBuildRangeInfoKHR>        rangeInfos{m_device->m_arena};
        TL::Vector<VkAccelerationStructureBuildGeometryInfoKHR>     geometryInfos{m_device->m_arena};
        TL::Vector<const VkAccelerationStructureBuildRangeInfoKHR*> pRangeInfos{m_device->m_arena};

        uint32_t totalGeometries = 0;
        for (const auto& info : buildInfos)
            totalGeometries += info.geometryCount;

        geometries.reserve(totalGeometries);
        rangeInfos.reserve(totalGeometries);
        geometryInfos.resize(buildInfos.size());
        pRangeInfos.resize(buildInfos.size());

        for (size_t i = 0; i < buildInfos.size(); ++i)
        {
            const auto& info       = buildInfos[i];
            auto*       dstAS      = (IAccelerationStructure*)info.dst;
            auto*       srcAS      = (IAccelerationStructure*)info.src;
            auto*       scratchBuf = (const IBuffer*)info.scratchBuffer;

            uint32_t firstGeometryIndex = (uint32_t)geometries.size();

            for (uint32_t g = 0; g < info.geometryCount; ++g)
            {
                const auto& geom = info.geometries[g];
                geometries.push_back(convertGeometryData(geom));

                uint32_t primitiveCount = 0;
                if (geom.geometryType == GeometryType::Triangles)
                    primitiveCount = (geom.indexCount > 0 ? geom.indexCount : geom.count) / 3;
                else
                    primitiveCount = geom.count;

                rangeInfos.push_back(VkAccelerationStructureBuildRangeInfoKHR{
                    .primitiveCount  = primitiveCount,
                    .primitiveOffset = 0,
                    .firstVertex     = 0,
                    .transformOffset = 0,
                });
            }

            geometryInfos[i] = VkAccelerationStructureBuildGeometryInfoKHR{
                .sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
                .pNext                    = nullptr,
                .type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
                .flags                    = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
                .mode                     = srcAS ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
                .srcAccelerationStructure = srcAS ? srcAS->handle : VK_NULL_HANDLE,
                .dstAccelerationStructure = dstAS->handle,
                .geometryCount            = info.geometryCount,
                .pGeometries              = geometries.data() + firstGeometryIndex,
                .ppGeometries             = nullptr,
                .scratchData              = {.deviceAddress = scratchBuf->address + info.scratchBufferOffset},
            };

            pRangeInfos[i] = rangeInfos.data() + firstGeometryIndex;
        }

        vkCmdBuildAccelerationStructuresKHR(m_commandBuffer, (uint32_t)geometryInfos.size(), geometryInfos.data(), pRangeInfos.data());
    }

    void ICommandList::BuildMicromaps(TL::Span<const MicromapBuildInfo> buildInfos)
    {
        TL_UNREACHABLE();
    }

    void ICommandList::WriteAccelerationStructuresSizes(TL::Span<const AccelerationStructure*> accelerationStructures, QueryPool* _queryPool, uint32_t queryPoolOffset)
    {
        IQueryPool*                            queryPool = (IQueryPool*)_queryPool;
        TL::Vector<VkAccelerationStructureKHR> asHandles{m_device->m_arena};
        asHandles.reserve(accelerationStructures.size());
        for (const auto* as : accelerationStructures)
        {
            auto vkAS = (IAccelerationStructure*)as;
            asHandles.push_back(vkAS->handle);
        }
        vkCmdWriteAccelerationStructuresPropertiesKHR(m_commandBuffer, (uint32_t)asHandles.size(), asHandles.data(), VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool->handle, queryPoolOffset);
    }

    void ICommandList::WriteMicromapsSizes(TL::Span<const Micromap*> micromaps, QueryPool* _queryPool, uint32_t queryPoolOffset)
    {
        IQueryPool*               queryPool = (IQueryPool*)_queryPool;
        TL::Vector<VkMicromapEXT> micromapHandles{m_device->m_arena};
        micromapHandles.reserve(micromaps.size());
        for (const auto* micromap : micromaps)
        {
            auto vkMicromap = (IMicromap*)micromap;
            micromapHandles.push_back(vkMicromap->handle);
        }
        vkCmdWriteMicromapsPropertiesEXT(m_commandBuffer, (uint32_t)micromapHandles.size(), micromapHandles.data(), VK_QUERY_TYPE_MICROMAP_COMPACTED_SIZE_EXT, queryPool->handle, queryPoolOffset);
    }
} // namespace RHI::Vulkan
