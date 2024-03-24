#include "CommandList.hpp"
#include "Common.hpp"
#include "Context.hpp"
#include "FrameScheduler.hpp"
#include "Resources.hpp"
#include "Swapchain.hpp"

#include <RHI/Attachments.hpp>
#include <RHI/Format.hpp>

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{
    enum class BarrierType
    {
        PrePass,
        PostPass,
    };

    struct BarrierStage
    {
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;
        VkImageLayout layout;
    };

    inline static bool IsSwapchainAttachment(const ImagePassAttachment& passAttachment)
    {
        return passAttachment.GetAttachment()->m_swapchain != nullptr;
    }

    inline static VkPipelineStageFlags2 GetPipelineStageFromShaderStage(Flags<ShaderStage> shader)
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

    inline static VkAccessFlags2 GetShaderAccessFlags(Access access, bool isStorage)
    {
        if (access == Access::Write)
            return isStorage ? VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT : VK_ACCESS_2_SHADER_WRITE_BIT;
        else if (access == Access::Read)
            return isStorage ? VK_ACCESS_2_SHADER_STORAGE_READ_BIT : VK_ACCESS_2_SHADER_READ_BIT;
        else if (access == Access::ReadWrite)
            return isStorage ? VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT : VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;

        return {};
    }

    inline static VkImageLayout GetImageLayout(ImageUsage usage, Access access)
    {
        (void)access;
        switch (usage)
        {
        case ImageUsage::Color:           return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageUsage::Depth:           return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        case ImageUsage::Stencil:         return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::DepthStencil:    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageUsage::CopySrc:         return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageUsage::CopyDst:         return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageUsage::ShaderResource:  return VK_IMAGE_LAYOUT_GENERAL;
        case ImageUsage::StorageResource: return VK_IMAGE_LAYOUT_GENERAL;
        default:                          RHI_UNREACHABLE(); return VK_IMAGE_LAYOUT_GENERAL;
        }
    }

    inline static BarrierStage GetImageTransitionInfo(BarrierType barrierType, const ImagePassAttachment* passAttachment)
    {
        if (passAttachment == nullptr)
        {
            return { barrierType == BarrierType::PrePass ? VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED };
        }

        auto usage = passAttachment->m_usage;
        auto access = passAttachment->m_access;
        auto stages = passAttachment->m_stage;
        auto loadStoreOperations = passAttachment->m_loadStoreOperations;

        VkAccessFlags2 renderTargetAccessFlags = {};
        if (loadStoreOperations.loadOperation == LoadOperation::Load)
            renderTargetAccessFlags |= usage == ImageUsage::Color ? VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        if (loadStoreOperations.storeOperation == StoreOperation::Store)
            renderTargetAccessFlags |= usage == ImageUsage::Color ? VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT : VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        // clang-format off
        switch (usage)
        {
        case ImageUsage::Color:           return { VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, renderTargetAccessFlags,             GetImageLayout(usage, access) };
        case ImageUsage::Depth:           return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    renderTargetAccessFlags,             GetImageLayout(usage, access) };
        case ImageUsage::Stencil:         return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    renderTargetAccessFlags,             GetImageLayout(usage, access) };
        case ImageUsage::DepthStencil:    return { VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,    renderTargetAccessFlags,             GetImageLayout(usage, access) };
        case ImageUsage::CopySrc:         return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,                VK_ACCESS_2_TRANSFER_READ_BIT,       GetImageLayout(usage, access) };
        case ImageUsage::CopyDst:         return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,                VK_ACCESS_2_TRANSFER_WRITE_BIT,      GetImageLayout(usage, access) };
        case ImageUsage::ShaderResource:  return { GetPipelineStageFromShaderStage(stages),         GetShaderAccessFlags(access, false), GetImageLayout(usage, access) };
        case ImageUsage::StorageResource: return { GetPipelineStageFromShaderStage(stages),         GetShaderAccessFlags(access, true),  GetImageLayout(usage, access) };
        default:                          RHI_UNREACHABLE(); return {};
        }
        // clang-format on
    }

    inline static BarrierStage GetBufferTransitionInfo(BarrierType barrierType, const BufferPassAttachment* passAttachment)
    {
        if (passAttachment == nullptr)
        {
            return { barrierType == BarrierType::PrePass ? VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_UNDEFINED };
        }

        auto usage = passAttachment->m_usage;
        auto access = passAttachment->m_access;
        auto stages = passAttachment->m_stage;

        // clang-format off
        switch (usage)
        {
        case BufferUsage::Storage: return { GetPipelineStageFromShaderStage(stages),  GetShaderAccessFlags(access, false), {} };
        case BufferUsage::Uniform: return { GetPipelineStageFromShaderStage(stages),  GetShaderAccessFlags(access, true),  {} };
        case BufferUsage::Vertex:  return { VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,     VK_ACCESS_2_SHADER_READ_BIT,         {} };
        case BufferUsage::Index:   return { VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT,      VK_ACCESS_2_SHADER_READ_BIT,         {} };
        case BufferUsage::CopySrc: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,         VK_ACCESS_2_TRANSFER_READ_BIT,       {} };
        case BufferUsage::CopyDst: return { VK_PIPELINE_STAGE_2_TRANSFER_BIT,         VK_ACCESS_2_TRANSFER_WRITE_BIT,      {} };
        default:                   RHI_UNREACHABLE(); return {};
        }
        // clang-format on
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    /// CommandList
    //////////////////////////////////////////////////////////////////////////////////////////

    ICommandList::ICommandList(IContext* context, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
        : m_commandBuffer(commandBuffer)
        , m_commandPool(commandPool)
        , m_context(context)
        , m_pass(nullptr)
    {
    }

    void ICommandList::Begin()
    {
        ZoneScoped;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    }

    void ICommandList::Begin(Pass& passBase)
    {
        ZoneScoped;

        auto& pass = static_cast<Pass&>(passBase);

        m_pass = &pass;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext = nullptr;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

        std::vector<VkImageMemoryBarrier2> imageBarriers;
        for (auto& passAttachment : m_pass->m_imagePassAttachments)
        {
            auto attachemnt = passAttachment->GetAttachment();
            auto srcPassAttachment = passAttachment->GetPrev();
            auto dstPassAttachment = passAttachment.get();
            auto image = m_context->m_imageOwner.Get(attachemnt->GetHandle());

            auto srcInfo = GetImageTransitionInfo(BarrierType::PrePass, srcPassAttachment);
            auto dstInfo = GetImageTransitionInfo(BarrierType::PrePass, dstPassAttachment);

            // if first use then collect all semaphore we need to wait on
            if (srcPassAttachment == nullptr)
            {
                // swapchain semaphore
                if (IsSwapchainAttachment(*passAttachment))
                {
                    auto swapchain = (ISwapchain*)attachemnt->m_swapchain;

                    auto& semaphoreInfo = m_waitSemaphores.emplace_back();
                    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    semaphoreInfo.semaphore = swapchain->GetImageReadySemaphore();
                    semaphoreInfo.stageMask = dstInfo.stage;
                }
                // transient resource nothing to do
                else if (image->waitSemaphore != VK_NULL_HANDLE)
                {
                    // resource is being streamed wait for it
                    auto& semaphoreInfo = m_waitSemaphores.emplace_back();
                    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    semaphoreInfo.semaphore = image->waitSemaphore;
                    semaphoreInfo.stageMask = dstInfo.stage;
                }
                else if (image->initalLayout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    continue;
                }
            }

            VkImageMemoryBarrier2& barrier = imageBarriers.emplace_back();
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.srcStageMask = srcInfo.stage;
            barrier.srcAccessMask = srcInfo.access;
            barrier.dstStageMask = dstInfo.stage;
            barrier.dstAccessMask = dstInfo.access;
            barrier.oldLayout = srcInfo.layout;
            barrier.newLayout = dstInfo.layout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image->handle;
            barrier.subresourceRange = ConvertSubresourceRange(passAttachment->m_viewInfo.subresource);
        }

        std::vector<VkBufferMemoryBarrier2> bufferBarriers;
        for (auto& passAttachment : m_pass->m_bufferPassAttachments)
        {
            auto attachemnt = passAttachment->GetAttachment();
            auto srcPassAttachment = passAttachment->GetPrev();
            auto dstPassAttachment = passAttachment.get();
            auto buffer = m_context->m_bufferOwner.Get(attachemnt->GetHandle());

            if (srcPassAttachment == nullptr)
            {
                continue;
            }

            auto srcInfo = GetBufferTransitionInfo(BarrierType::PrePass, srcPassAttachment);
            auto dstInfo = GetBufferTransitionInfo(BarrierType::PrePass, dstPassAttachment);

            VkBufferMemoryBarrier2& barrier = bufferBarriers.emplace_back();
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier.pNext = nullptr;
            barrier.srcStageMask = srcInfo.stage;
            barrier.srcAccessMask = srcInfo.access;
            barrier.dstStageMask = dstInfo.stage;
            barrier.dstAccessMask = dstInfo.access;
            barrier.buffer = buffer->handle;
            barrier.offset = passAttachment->m_viewInfo.byteOffset;
            barrier.size = passAttachment->m_viewInfo.byteSize;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }

        PipelineBarrier({}, bufferBarriers, imageBarriers);

        if (m_pass && m_pass->GetQueueType() == QueueType::Graphics)
        {
            std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos;
            VkRenderingAttachmentInfo depthAttachmentInfo = {};
            bool hasDepthAttachment = false;

            for (auto passAttachment : m_pass->GetColorAttachments())
            {
                auto view = m_context->m_imageViewOwner.Get(passAttachment->m_view);

                VkRenderingAttachmentInfo& attachmentInfo = colorAttachmentInfos.emplace_back();
                attachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                attachmentInfo.pNext = nullptr;
                attachmentInfo.imageView = view->handle;
                attachmentInfo.imageLayout = GetImageLayout(passAttachment->m_usage, Access::None);
                attachmentInfo.loadOp = ConvertLoadOp(passAttachment->m_loadStoreOperations.loadOperation);
                attachmentInfo.storeOp = ConvertStoreOp(passAttachment->m_loadStoreOperations.storeOperation);
                attachmentInfo.clearValue.color = ConvertColorValue(passAttachment->m_clearValue.colorValue);
            }

            if (auto passAttachment = m_pass->GetDepthStencilAttachment(); passAttachment != nullptr)
            {
                auto view = m_context->m_imageViewOwner.Get(passAttachment->m_view);

                depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
                depthAttachmentInfo.pNext = nullptr;
                depthAttachmentInfo.imageView = view->handle;
                depthAttachmentInfo.imageLayout = GetImageLayout(passAttachment->m_usage, Access::None);
                depthAttachmentInfo.loadOp = ConvertLoadOp(passAttachment->m_loadStoreOperations.loadOperation);
                depthAttachmentInfo.storeOp = ConvertStoreOp(passAttachment->m_loadStoreOperations.storeOperation);
                depthAttachmentInfo.clearValue.depthStencil = ConvertDepthStencilValue(passAttachment->m_clearValue.depthStencilValue);
                hasDepthAttachment = true;
            }

            RenderingBegin(
                colorAttachmentInfos,
                hasDepthAttachment ? &depthAttachmentInfo : nullptr,
                m_pass->m_frameSize);
        }
    }

    void ICommandList::End()
    {
        ZoneScoped;

        if (m_pass && m_pass->GetQueueType() == QueueType::Graphics)
        {
            RenderingEnd();
        }

        if (m_pass)
        {
            std::vector<VkImageMemoryBarrier2> imageBarriers;
            for (auto& passAttachment : m_pass->m_imagePassAttachments)
            {
                auto attachemnt = passAttachment->GetAttachment();
                auto srcPassAttachment = passAttachment.get();
                auto dstPassAttachment = passAttachment->GetNext();
                auto image = m_context->m_imageOwner.Get(attachemnt->GetHandle());

                auto srcInfo = GetImageTransitionInfo(BarrierType::PostPass, srcPassAttachment);
                auto dstInfo = GetImageTransitionInfo(BarrierType::PostPass, dstPassAttachment);

                if (passAttachment->GetNext() == nullptr && IsSwapchainAttachment(*passAttachment.get()))
                {
                    auto swapchain = (ISwapchain*)attachemnt->m_swapchain;

                    dstInfo = { VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };

                    auto& signalSemaphore = m_signalSemaphores.emplace_back();
                    signalSemaphore.semaphore = swapchain->GetFrameReadySemaphore();
                    signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    signalSemaphore.stageMask = dstInfo.stage;
                }
                // a read operation is staged
                else if (image->signalSemaphore != VK_NULL_HANDLE)
                {
                    auto& signalSemaphore = m_signalSemaphores.emplace_back();
                    signalSemaphore.semaphore = image->signalSemaphore;
                    signalSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
                    signalSemaphore.stageMask = dstInfo.stage;
                }
                else
                {
                    continue;
                }

                VkImageMemoryBarrier2& barrier = imageBarriers.emplace_back();
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                barrier.pNext = nullptr;
                barrier.srcStageMask = srcInfo.stage;
                barrier.srcAccessMask = srcInfo.access;
                barrier.dstStageMask = dstInfo.stage;
                barrier.dstAccessMask = dstInfo.access;
                barrier.oldLayout = srcInfo.layout;
                barrier.newLayout = dstInfo.layout;
                barrier.image = image->handle;
                barrier.subresourceRange = ConvertSubresourceRange(passAttachment->m_viewInfo.subresource);
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }

            std::vector<VkBufferMemoryBarrier2> bufferBarriers;
            for (auto& passAttachment : m_pass->m_bufferPassAttachments)
            {
                auto attachemnt = passAttachment->GetAttachment();
                auto srcPassAttachment = passAttachment.get();
                auto dstPassAttachment = passAttachment->GetNext();
                auto buffer = m_context->m_bufferOwner.Get(attachemnt->GetHandle());

                auto srcInfo = GetBufferTransitionInfo(BarrierType::PostPass, srcPassAttachment);
                auto dstInfo = GetBufferTransitionInfo(BarrierType::PostPass, dstPassAttachment);

                VkBufferMemoryBarrier2& barrier = bufferBarriers.emplace_back();
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
                barrier.pNext = nullptr;
                barrier.srcStageMask = srcInfo.stage;
                barrier.srcAccessMask = srcInfo.access;
                barrier.dstStageMask = dstInfo.stage;
                barrier.dstAccessMask = dstInfo.access;
                barrier.buffer = buffer->handle;
                barrier.offset = passAttachment->m_viewInfo.byteOffset;
                barrier.size = passAttachment->m_viewInfo.byteSize;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }

            PipelineBarrier({}, bufferBarriers, imageBarriers);
        }

        vkEndCommandBuffer(m_commandBuffer);
    }

    void ICommandList::DebugMarkerPush(const char* name, const ColorValue& color)
    {
        ZoneScoped;

        (void)name;
        (void)color;

#if RHI_DEBUG
        if (m_context->m_vkCmdDebugMarkerBeginEXT)
        {
            VkDebugMarkerMarkerInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
            info.pNext = nullptr;
            info.pMarkerName = name;
            info.color[0] = color.r;
            info.color[1] = color.g;
            info.color[2] = color.b;
            info.color[3] = color.a;
            m_context->m_vkCmdDebugMarkerBeginEXT(m_commandBuffer, &info);
        }
#endif
    }

    void ICommandList::DebugMarkerPop()
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_context->m_vkCmdDebugMarkerEndEXT)
        {
            m_context->m_vkCmdDebugMarkerEndEXT(m_commandBuffer);
        }
#endif
    }

    void ICommandList::BeginConditionalCommands(Handle<Buffer> handle, size_t offset, bool inverted)
    {
        ZoneScoped;

        if (m_context->m_vkCmdBeginConditionalRenderingEXT)
        {
            m_context->DebugLogWarn("This function is not available");
            return;
        }

        auto buffer = m_context->m_bufferOwner.Get(handle);

        VkConditionalRenderingBeginInfoEXT beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
        beginInfo.pNext = nullptr;
        beginInfo.buffer = buffer->handle;
        beginInfo.offset = offset;
        beginInfo.flags = inverted ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : 0u;
        m_context->m_vkCmdBeginConditionalRenderingEXT(m_commandBuffer, &beginInfo);
    }

    void ICommandList::EndConditionalCommands()
    {
        ZoneScoped;

        if (m_context->m_vkCmdEndConditionalRenderingEXT)
        {
            m_context->DebugLogWarn("This function is not available");
            return;
        }

        m_context->m_vkCmdEndConditionalRenderingEXT(m_commandBuffer);
    }

    void ICommandList::Execute(TL::Span<const CommandList*> commandLists)
    {
        ZoneScoped;

        TL::Vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(commandLists.size());
        for (auto _commandList : commandLists)
        {
            auto commandList = (const ICommandList*)_commandList;
            commandBuffers.push_back(commandList->m_commandBuffer);
        }
        vkCmdExecuteCommands(m_commandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());
    }

    void ICommandList::SetViewport(const Viewport& viewport)
    {
        ZoneScoped;

        VkViewport vkViewport{};
        vkViewport.x = viewport.offsetX;
        vkViewport.y = viewport.offsetY;
        vkViewport.width = viewport.width;
        vkViewport.height = viewport.height;
        vkViewport.minDepth = viewport.minDepth;
        vkViewport.maxDepth = viewport.maxDepth;
        vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
    }

    void ICommandList::SetSicssor(const Scissor& scissor)
    {
        ZoneScoped;

        VkRect2D vkScissor{};
        vkScissor.extent.width = scissor.width;
        vkScissor.extent.height = scissor.height;
        vkScissor.offset.x = scissor.offsetX;
        vkScissor.offset.y = scissor.offsetY;
        vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
    }

    void ICommandList::Draw(const DrawInfo& command)
    {
        ZoneScoped;

        auto pipeline = m_context->m_graphicsPipelineOwner.Get(command.pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);

        if (command.bindGroups.empty() == false)
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout, command.bindGroups);
        }

        if (command.vertexBuffers.empty() == false)
        {
            uint32_t vertexBuffersCount = 0;
            VkBuffer vertexBuffers[c_MaxPipelineVertexBindings] = {};
            VkDeviceSize vertexBufferOffsets[c_MaxPipelineVertexBindings] = {};
            for (auto handle : command.vertexBuffers)
            {
                auto buffer = m_context->m_bufferOwner.Get(handle);
                auto index = vertexBuffersCount++;
                vertexBuffers[index] = buffer->handle;
                vertexBufferOffsets[index] = 0;
            }
            vkCmdBindVertexBuffers(m_commandBuffer, 0, vertexBuffersCount, vertexBuffers, vertexBufferOffsets);
        }

        auto parameters = command.parameters;
        if (command.indexBuffers)
        {
            auto buffer = m_context->m_bufferOwner.Get(command.indexBuffers);
            vkCmdBindIndexBuffer(m_commandBuffer, buffer->handle, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(m_commandBuffer, parameters.elementCount, parameters.instanceCount, parameters.firstElement, int32_t(parameters.vertexOffset), uint32_t(parameters.firstInstance));
        }
        else
        {
            vkCmdDraw(m_commandBuffer, parameters.elementCount, parameters.instanceCount, parameters.firstElement, uint32_t(parameters.firstInstance));
        }
    }

    void ICommandList::Dispatch(const DispatchInfo& command)
    {
        ZoneScoped;

        auto pipeline = m_context->m_computePipelineOwner.Get(command.pipelineState);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->handle);
        if (command.bindGroups.size())
        {
            BindShaderBindGroups(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->layout, command.bindGroups);
        }
        auto parameters = command.parameters;
        vkCmdDispatchBase(m_commandBuffer, parameters.offsetX, parameters.offsetY, parameters.offsetZ, parameters.countX, parameters.countY, parameters.countZ);
    }

    void ICommandList::Copy(const BufferCopyInfo& command)
    {
        ZoneScoped;

        auto srcBuffer = m_context->m_bufferOwner.Get(command.srcBuffer);
        auto destinationBuffer = m_context->m_bufferOwner.Get(command.dstBuffer);

        auto copyInfo = VkBufferCopy{};
        copyInfo.srcOffset = command.srcOffset;
        copyInfo.dstOffset = command.dstOffset;
        copyInfo.size = command.size;
        vkCmdCopyBuffer(m_commandBuffer, srcBuffer->handle, destinationBuffer->handle, 1, &copyInfo);
    }

    void ICommandList::Copy(const ImageCopyInfo& command)
    {
        ZoneScoped;

        auto srcImage = m_context->m_imageOwner.Get(command.srcImage);
        auto dstImage = m_context->m_imageOwner.Get(command.dstImage);

        auto copyInfo = VkImageCopy{};
        copyInfo.srcSubresource = ConvertSubresourceLayer(command.srcSubresource);
        copyInfo.srcOffset = ConvertOffset3D(command.srcOffset);
        copyInfo.dstSubresource = ConvertSubresourceLayer(command.dstSubresource);
        copyInfo.dstOffset = ConvertOffset3D(command.dstOffset);
        copyInfo.extent = ConvertExtent3D(command.srcSize);
        vkCmdCopyImage(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
    }

    void ICommandList::Copy(const BufferToImageCopyInfo& command)
    {
        ZoneScoped;

        auto srcBuffer = m_context->m_bufferOwner.Get(command.srcBuffer);
        auto dstImage = m_context->m_imageOwner.Get(command.dstImage);

        /// @todo: remove those fixed barriers

        VkImageMemoryBarrier2 transitionImage{};
        transitionImage.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        transitionImage.pNext = nullptr;
        transitionImage.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.srcAccessMask = 0;
        transitionImage.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        transitionImage.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        transitionImage.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transitionImage.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transitionImage.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transitionImage.image = dstImage->handle;
        auto layer = ConvertSubresourceLayer(command.dstSubresource);
        transitionImage.subresourceRange.aspectMask = layer.aspectMask;
        transitionImage.subresourceRange.levelCount = 1;
        transitionImage.subresourceRange.layerCount = 1;

        VkDependencyInfo barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        barrier.imageMemoryBarrierCount = 1;
        barrier.pImageMemoryBarriers = &transitionImage;

        vkCmdPipelineBarrier2(m_commandBuffer, &barrier);

        auto copyInfo = VkBufferImageCopy{};
        copyInfo.bufferOffset = command.srcOffset;
        copyInfo.bufferRowLength = command.srcBytesPerRow;
        // copyInfo.bufferImageHeight;
        copyInfo.imageSubresource = ConvertSubresourceLayer(command.dstSubresource);
        copyInfo.imageOffset = ConvertOffset3D(command.dstOffset);
        copyInfo.imageExtent = ConvertExtent3D(command.srcSize);
        vkCmdCopyBufferToImage(m_commandBuffer, srcBuffer->handle, dstImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        transitionImage.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        transitionImage.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        transitionImage.dstAccessMask = 0;
        transitionImage.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transitionImage.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        vkCmdPipelineBarrier2(m_commandBuffer, &barrier);
    }

    void ICommandList::Copy(const ImageToBufferCopyInfo& command)
    {
        ZoneScoped;

        auto srcImage = m_context->m_imageOwner.Get(command.srcImage);
        auto dstBuffer = m_context->m_bufferOwner.Get(command.dstBuffer);

        auto copyInfo = VkBufferImageCopy{};
        copyInfo.bufferOffset = command.dstOffset;
        copyInfo.bufferRowLength = command.dstBytesPerRow;
        // copyInfo.bufferImageHeight;
        copyInfo.imageSubresource = ConvertSubresourceLayer(command.srcSubresource);
        copyInfo.imageOffset = ConvertOffset3D(command.srcOffset);
        copyInfo.imageExtent = ConvertExtent3D(command.srcSize);
        vkCmdCopyImageToBuffer(m_commandBuffer, srcImage->handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstBuffer->handle, 1, &copyInfo);
    }

    void ICommandList::BindShaderBindGroups(VkPipelineBindPoint bindPoint, VkPipelineLayout pipelineLayout, TL::Span<Handle<BindGroup>> bindGroups)
    {
        uint32_t count = 0;
        VkDescriptorSet descriptorSets[c_MaxPipelineBindGroupsCount] = {};
        for (auto bindGroupHandle : bindGroups)
        {
            auto bindGroup = m_context->m_bindGroupOwner.Get(bindGroupHandle);
            descriptorSets[count++] = bindGroup->descriptorSet;
        }
        vkCmdBindDescriptorSets(m_commandBuffer, bindPoint, pipelineLayout, 0, count, descriptorSets, 0, nullptr);
    }

    void ICommandList::RenderingBegin(
        TL::Span<const VkRenderingAttachmentInfo> attachmentInfos,
        const VkRenderingAttachmentInfo* depthAttachmentInfo,
        ImageSize2D extent)
    {
        ZoneScoped;

        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.pNext = nullptr;
        renderingInfo.flags = 0;
        renderingInfo.renderArea.extent = ConvertExtent2D(extent);
        renderingInfo.renderArea.offset = ConvertOffset2D({ 0u, 0u, 0u });
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = uint32_t(attachmentInfos.size());
        renderingInfo.pColorAttachments = attachmentInfos.data();
        renderingInfo.pDepthAttachment = depthAttachmentInfo;
        vkCmdBeginRendering(m_commandBuffer, &renderingInfo);
    }

    void ICommandList::RenderingEnd()
    {
        ZoneScoped;
        vkCmdEndRendering(m_commandBuffer);
    }

    void ICommandList::PipelineBarrier(
        TL::Span<const VkMemoryBarrier2> memoryBarriers,
        TL::Span<const VkBufferMemoryBarrier2> bufferBarriers,
        TL::Span<const VkImageMemoryBarrier2> imageBarriers)
    {
        VkDependencyInfo dependencyInfo{};
        dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependencyInfo.pNext = nullptr;
        dependencyInfo.dependencyFlags = 0;
        dependencyInfo.memoryBarrierCount = uint32_t(memoryBarriers.size());
        dependencyInfo.pMemoryBarriers = memoryBarriers.data();
        dependencyInfo.bufferMemoryBarrierCount = uint32_t(bufferBarriers.size());
        dependencyInfo.pBufferMemoryBarriers = bufferBarriers.data();
        dependencyInfo.imageMemoryBarrierCount = uint32_t(imageBarriers.size());
        dependencyInfo.pImageMemoryBarriers = imageBarriers.data();
        vkCmdPipelineBarrier2(m_commandBuffer, &dependencyInfo);
    }

} // namespace RHI::Vulkan
