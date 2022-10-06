#include "Backend/Vulkan/Commands.hpp"
#include "Backend/Vulkan/Common.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/PipelineState.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{

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

    void CommandBuffer::SetViewports(const std::vector<Viewport>& viewports)
    {
        std::vector<VkViewport> vkViewports;
        std::transform(viewports.begin(), viewports.end(), std::back_inserter(vkViewports), [](Viewport viewport) { return ConvertViewport(viewport); });
        vkCmdSetViewport(m_handle, 0, CountElements(viewports), vkViewports.data());
    }

    void CommandBuffer::SetScissors(const std::vector<Rect>& scissors)
    {
        std::vector<VkRect2D> scissorRects;
        std::transform(scissors.begin(), scissors.end(), std::back_inserter(scissorRects), [](Rect rect) { return ConvertRect(rect); });
        vkCmdSetScissor(m_handle, 0, CountElements(scissors), scissorRects.data());
    }

    void CommandBuffer::Submit(const DrawCommand& drawCommand)
    {
        VkRenderPassBeginInfo renderPassBegin;
        VkSubpassBeginInfo    subpassBeginInfo;

        vkCmdBeginRenderPass2(m_handle, &renderPassBegin, &subpassBeginInfo);
        const PipelineState& pso = *static_cast<const PipelineState*>(drawCommand.pPipelineState);

        vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetHandle());

        std::vector<VkDescriptorSet> descriptorSets;
        vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pso.GetLayout().GetHandle(), 0, CountElements(descriptorSets), descriptorSets.data(),
                                0, nullptr);

        switch (drawCommand.type)
        {
        case RHI::DrawCommand::EType::Indexed:
        {
            break;
        }
        case RHI::DrawCommand::EType::Linear:
        {
            break;
        }
        }
        VkSubpassEndInfo endInfo;
        vkCmdEndRenderPass2(m_handle, &endInfo);
    }

    void CommandBuffer::Submit(const CopyCommand& copyCommand)
    {
        if (copyCommand.destinationResourceType == EResourceType::Buffer && copyCommand.sourceResourceType == EResourceType::Buffer)
        {
            Buffer&           srcBuffer = static_cast<Buffer&>(*copyCommand.srcBuffer.pBuffer);
            Buffer&           dstBuffer = static_cast<Buffer&>(*copyCommand.dstBuffer.pBuffer);
            VkCopyBufferInfo2 copyInfo;
            // copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
            // copyInfo.pNext = nullptr;
            // copyInfo.srcBuffer = srcBuffer.GetHandle();
            // copyInfo.dstBuffer = dstBuffer.GetHandle();
            // copyInfo.regionCount;
            // copyInfo.pRegions;
        }
        else if (copyCommand.destinationResourceType == EResourceType::Buffer && copyCommand.sourceResourceType == EResourceType::Image)
        {
            VkCopyImageToBufferInfo2 copyInfo;
        }
        else if (copyCommand.destinationResourceType == EResourceType::Image && copyCommand.sourceResourceType == EResourceType::Image)
        {
            VkCopyImageInfo2 copyInfo;
        }
        else if (copyCommand.destinationResourceType == EResourceType::Image && copyCommand.sourceResourceType == EResourceType::Buffer)
        {
            VkCopyImageToBufferInfo2 copyInfo;
        }
    }

    void CommandBuffer::Submit(const DispatchCommand& dispatchCommand) {}

    CommandAllocator::~CommandAllocator()
    {
        vkDestroyCommandPool(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult CommandAllocator::Init(ECommandPrimaryTask task)
    {
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;

        switch (task)
        {
        case ECommandPrimaryTask::Graphics: createInfo.queueFamilyIndex = m_pDevice->GetTransferQueue().GetFamilyIndex(); break;
        case ECommandPrimaryTask::Compute: createInfo.queueFamilyIndex = m_pDevice->GetTransferQueue().GetFamilyIndex(); break;
        case ECommandPrimaryTask::Transfer: createInfo.queueFamilyIndex = m_pDevice->GetTransferQueue().GetFamilyIndex(); break;
        }

        return vkCreateCommandPool(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    VkResult CommandAllocator::Reset()
    {
        return vkResetCommandPool(m_pDevice->GetHandle(), m_handle, 0);
    }

    Unique<CommandBuffer> CommandAllocator::AllocateCommandBuffer()
    {
        return nullptr;
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
        vkAllocateCommandBuffers(m_pDevice->GetHandle(), &allocateInfo, handles.data());

        for (VkCommandBuffer handle : handles)
        {
            commandBuffers.emplace_back(this, handle);
        }

        return commandBuffers;
    }

} // namespace Vulkan
} // namespace RHI