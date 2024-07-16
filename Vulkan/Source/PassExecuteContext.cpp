#include <RHI/RenderGraph.hpp>

#include "Context.hpp"

#include "PassExecuteContext.hpp"

#include "CommandList.hpp"

namespace RHI::Vulkan
{
    PassExecuteContext::PassExecuteContext(RenderGraph& renderGraph, Handle<Pass> _pass, TL::Span<const LoadStoreOperations> loadStoreOperations)
        : m_renderGraph(&renderGraph)
        , m_pass(_pass)
    {
        auto pass = m_renderGraph->m_passPool.Get(_pass);

        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> signalSemaphores;
        TL::UnorderedMap<VkSemaphore, VkPipelineStageFlags2> waitSemaphores;

        uint32_t targetAttachmentIndex = 0;
        for (auto passAttachment : pass->GetImageAttachments())
        {
            auto prevAttachment = if (auto it = passAttachment->it++; it != pass);
            auto nextAttachment = if (auto it = passAttachment->it--; it != pass);

            auto imageHandle = m_renderGraph->GetImage(passAttachment->attachment);
            auto image = m_context->m_imageOwner.Get(imageHandle);
            auto subresourceRange = ConvertSubresourceRange(passAttachment->viewInfo.subresourceLayers);

            ImageStageAccess prilogeState = image->initialState.pipelineStage;
            ImageStageAccess epilogeState = image->finalState.pipelineStage;
            ImageStageAccess currentState = GetImageStageAccess(passAttachment->usage, passAttachment->access, passAttachment->shaderStages, loadStoreOperations[targetAttachmentIndex]);

            if (prevAttachment)
            {
                auto usage = passAttachment->usage;
                auto access = passAttachment->access;
                auto stages = passAttachment->shaderStages;
                prilogeState = GetImageStageAccess(usage, access, stages, loadStoreOperations[targetAttachmentIndex]);

                for (auto semaphore : image->initialState.semaphores)
                {
                    waitSemaphores[semaphore.semaphore] = semaphore.stageMask;
                }
            }

            if (nextAttachment)
            {
                auto usage = passAttachment->usage;
                auto access = passAttachment->access;
                auto stages = passAttachment->shaderStages;
                epilogeState = GetImageStageAccess(usage, access, stages, loadStoreOperations[targetAttachmentIndex]);

                for (auto semaphore : image->finalState.semaphores)
                {
                    waitSemaphores[semaphore.semaphore] = semaphore.stageMask;
                }
            }

            if (prilogeState != currentState)
            {
                m_barriers[BarrierSlot::Priloge].imageBarriers.emplace_back(CreateImageBarrier(image->handle, subresourceRange, prilogeState, currentState));
            }

            if (epilogeState != currentState)
            {
                m_barriers[BarrierSlot::Epiloge].imageBarriers.emplace_back(CreateImageBarrier(image->handle, subresourceRange, currentState, epilogeState));
            }

            if ((passAttachment->usage & ImageUsage::Color) || (passAttachment->usage & ImageUsage::DepthStencil))
            {
                targetAttachmentIndex++;
            }
        }

        for (auto passAttachment : pass->GetBufferAttachments())
        {
            auto prevAttachment = passAttachment->prev;
            auto nextAttachment = passAttachment->next;

            auto bufferHandle = m_renderGraph->GetBuffer(passAttachment->attachment);
            auto buffer = m_context->m_bufferOwner.Get(bufferHandle);
            auto subregion = passAttachment->viewInfo.subregion;

            BufferStageAccess prilogeState = buffer->initialState.pipelineStage;
            BufferStageAccess epilogeState = buffer->finalState.pipelineStage;
            BufferStageAccess currentState = GetBufferStageAccess(passAttachment->usage, passAttachment->access, passAttachment->shaderStages);

            if (prevAttachment)
            {
                auto usage = passAttachment->usage;
                auto access = passAttachment->access;
                auto stages = passAttachment->shaderStages;
                prilogeState = GetBufferStageAccess(usage, access, stages);

                for (auto semaphore : buffer->initialState.semaphores)
                {
                    waitSemaphores[semaphore.semaphore] = semaphore.stageMask;
                }
            }

            if (nextAttachment)
            {
                auto usage = passAttachment->usage;
                auto access = passAttachment->access;
                auto stages = passAttachment->shaderStages;
                epilogeState = GetBufferStageAccess(usage, access, stages);

                for (auto semaphore : buffer->finalState.semaphores)
                {
                    waitSemaphores[semaphore.semaphore] = semaphore.stageMask;
                }
            }

            if (prilogeState != currentState)
            {
                m_barriers[BarrierSlot::Priloge].bufferBarriers.emplace_back(CreateBufferBarrier(buffer->handle, subregion, prilogeState, currentState));
            }

            if (epilogeState != currentState)
            {
                m_barriers[BarrierSlot::Epiloge].bufferBarriers.emplace_back(CreateBufferBarrier(buffer->handle, subregion, currentState, epilogeState));
            }
        }

        if (pass->flags & PassFlags::Graphics)
        {
            uint32_t index = 0;
            for (auto colorAttachment : pass->GetColorAttachments())
            {
                auto imageViewHandle = m_renderGraph->PassGetImageView(colorAttachment->pass, colorAttachment->attachment);
                auto imageView = m_context->m_imageViewOwner.Get(imageViewHandle);
                auto attachmentInfo = CreateColorAttachment(imageView->handle, loadStoreOperations[index++]);
                m_colorAttachments.push_back(attachmentInfo);
            }

            if (auto depthStencilAttachment = pass->GetDepthStencilAttachment())
            {
                auto imageViewHandle = m_renderGraph->PassGetImageView(depthStencilAttachment->pass, depthStencilAttachment->attachment);
                auto imageView = m_context->m_imageViewOwner.Get(imageViewHandle);

                if (depthStencilAttachment->usage & ImageUsage::Depth)
                {
                    m_depthAttachment = CreateDepthAttachment(imageView->handle, loadStoreOperations.back());
                }

                if (depthStencilAttachment->usage & ImageUsage::Stencil)
                {
                    m_stencilAttachment = CreateStencilAttachment(imageView->handle, loadStoreOperations.back());
                }
            }
        }

        for (auto [semaphore, stageMask] : signalSemaphores)
        {
            VkSemaphoreSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            submitInfo.semaphore = semaphore;
            submitInfo.stageMask = stageMask;
            m_signalSemaphores.push_back(submitInfo);
        }

        for (auto [semaphore, stageMask] : waitSemaphores)
        {
            VkSemaphoreSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            submitInfo.semaphore = semaphore;
            submitInfo.stageMask = stageMask;
            m_waitSemaphores.push_back(submitInfo);
        }
    }

    void PassExecuteContext::Begin(ICommandList& commandList)
    {
        auto& barriers = m_barriers[BarrierSlot::Priloge];
        commandList.PipelineBarrier(barriers.memoryBarriers, barriers.bufferBarriers, barriers.imageBarriers);

        if (m_flags & PassFlags::Graphics)
        {
            RenderingBeginInfo beginInfo{};
            beginInfo.colorAttachments = m_colorAttachments;
            beginInfo.depthAttachment = m_depthAttachment.has_value() ? &m_depthAttachment.value() : nullptr;
            beginInfo.stencilAttachment = m_stencilAttachment.has_value() ? &m_stencilAttachment.value() : nullptr;
            beginInfo.renderingArea = m_renderArea;
            commandList.BeginRendering(beginInfo);
        }
    }

    void PassExecuteContext::End(ICommandList& commandList)
    {
        if (m_flags & PassFlags::Graphics)
        {
            commandList.EndRendedring();
        }

        auto& barriers = m_barriers[BarrierSlot::Priloge];
        commandList.PipelineBarrier(barriers.memoryBarriers, barriers.bufferBarriers, barriers.imageBarriers);
    }

} // namespace RHI::Vulkan