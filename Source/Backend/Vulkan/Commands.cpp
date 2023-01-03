#include "RHI/Pch.hpp"

#include "Backend/Vulkan/Common.hpp"

#include "Backend/Vulkan/Commands.hpp"

#include "Backend/Vulkan/Buffer.hpp"
#include "Backend/Vulkan/CommandQueue.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Framebuffer.hpp"
#include "Backend/Vulkan/Image.hpp"
#include "Backend/Vulkan/PipelineState.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::Begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext                    = nullptr;
    beginInfo.flags                    = 0;
    beginInfo.pInheritanceInfo         = nullptr;

    vkBeginCommandBuffer(m_handle, &beginInfo);
}

void CommandBuffer::End()
{
    vkEndCommandBuffer(m_handle);
}

void CommandBuffer::BeginRenderPass(Framebuffer& framebuffer)
{
    VkRect2D renderArea {};
    renderArea.offset = {};
    renderArea.extent = framebuffer.m_extent;

    VkClearValue clearValue;
    clearValue.color.float32[0] = 0.3f;
    clearValue.color.float32[1] = 0.6f;
    clearValue.color.float32[2] = 0.9f;
    clearValue.color.float32[3] = 1.0f;

    std::vector<VkClearValue> clearValues = {clearValue};  // = framebuffer.GetLayout().GetClearValues();

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext                 = nullptr;
    beginInfo.renderPass            = framebuffer.GetLayout().GetHandle();
    beginInfo.framebuffer           = framebuffer.GetHandle();
    beginInfo.renderArea            = renderArea;
    beginInfo.clearValueCount       = CountElements(clearValues);
    beginInfo.pClearValues          = clearValues.data();
    vkCmdBeginRenderPass(m_handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(m_handle);
}

void CommandBuffer::SetViewports(std::span<const Viewport> viewports)
{
    std::vector<VkViewport> vkViewports;
    std::transform(
        viewports.begin(), viewports.end(), std::back_inserter(vkViewports), [](Viewport viewport) { return ConvertViewport(viewport); });
    vkCmdSetViewport(m_handle, 0, CountElements(viewports), vkViewports.data());
}

void CommandBuffer::SetScissors(std::span<const Rect> scissors)
{
    std::vector<VkRect2D> scissorRects;
    std::transform(scissors.begin(), scissors.end(), std::back_inserter(scissorRects), [](Rect rect) { return ConvertRect(rect); });
    vkCmdSetScissor(m_handle, 0, CountElements(scissors), scissorRects.data());
}

template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

void CommandBuffer::Submit(const DrawCommand& drawCommand)
{
    vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<PipelineState&>(*drawCommand.pPipelineState).GetHandle());

    auto indexedDrawCommand = [&](RHI::DrawCommand::IndexedDrawData drawData)
    {
        VkBuffer vertexBuffer = static_cast<Buffer&>(*drawCommand.pVertexBuffer).GetHandle();
        VkBuffer indexBuffer  = static_cast<Buffer&>(*drawCommand.pIndexBuffer).GetHandle();
        size_t   offset       = 0;
        vkCmdBindVertexBuffers(m_handle, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(m_handle, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(m_handle,
                         drawData.indexCount,
                         drawData.instanceCount,
                         drawData.indexOffset,
                         static_cast<int32_t>(drawData.vertexOffset),
                         drawData.instanceOffset);
    };

    auto linearDrawCommand = [&](RHI::DrawCommand::LinearDrawData drawData)
    {
        VkBuffer vertexBuffer = static_cast<Buffer&>(*drawCommand.pVertexBuffer).GetHandle();
        size_t   offset       = 0;
        vkCmdBindVertexBuffers(m_handle, 0, 1, &vertexBuffer, &offset);

        vkCmdDraw(m_handle, drawData.vertexOffset, drawData.instanceCount, drawData.vertexOffset, drawData.instanceOffset);
    };

    std::visit(overloaded {indexedDrawCommand, linearDrawCommand}, drawCommand.drawData);
}

CommandAllocator::~CommandAllocator()
{
    vkDestroyCommandPool(m_device->GetHandle(), m_handle, nullptr);
}

VkResult CommandAllocator::Init()
{
    VkCommandPoolCreateInfo createInfo;
    createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.pNext            = nullptr;
    createInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = m_device->GetGraphicsQueue().m_familyIndex;

    return vkCreateCommandPool(m_device->GetHandle(), &createInfo, nullptr, &m_handle);
}

VkResult CommandAllocator::Reset()
{
    return vkResetCommandPool(m_device->GetHandle(), m_handle, 0);
}

Unique<CommandBuffer> CommandAllocator::AllocateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext                       = nullptr;
    allocateInfo.commandPool                 = m_handle;
    allocateInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount          = 1;

    VkCommandBuffer handle = VK_NULL_HANDLE;
    vkAllocateCommandBuffers(m_device->GetHandle(), &allocateInfo, &handle);

    return CreateUnique<CommandBuffer>(*m_device, this, handle);
}

std::vector<CommandBuffer> CommandAllocator::AllocateCommandBuffers(uint32_t count)
{
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.pNext                       = nullptr;
    allocateInfo.commandPool                 = m_handle;
    allocateInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount          = count;

    std::vector<VkCommandBuffer> handles(count, VK_NULL_HANDLE);
    std::vector<CommandBuffer>   commandBuffers;
    commandBuffers.reserve(count);
    vkAllocateCommandBuffers(m_device->GetHandle(), &allocateInfo, handles.data());

    for (VkCommandBuffer handle : handles)
    {
        commandBuffers.emplace_back(*m_device, this, handle);
    }

    return commandBuffers;
}

}  // namespace Vulkan
}  // namespace RHI