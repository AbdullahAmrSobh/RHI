#include "RHI/Backend/Vulkan/CommandList.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

#include "RHI/Backend/Vulkan/DescriptorSet.hpp"
#include "RHI/Backend/Vulkan/PipelineLayout.hpp"
#include "RHI/Backend/Vulkan/PipelineState.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Image.hpp"

namespace RHI
{
namespace Vulkan
{
    CommandList::~CommandList() {}

    void CommandList::Reset()
    {
        vkResetCommandBuffer(m_handle, 0);
    }

    void CommandList::Begin()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = 0;
        beginInfo.pNext                    = nullptr;
        beginInfo.pInheritanceInfo         = nullptr;
        vkBeginCommandBuffer(m_handle, &beginInfo);
    }

    void CommandList::End()
    {
        vkEndCommandBuffer(m_handle);
    }

    void CommandList::SetViewports(Span<const Viewport> viewports)
    {
        std::vector<VkViewport> vkViewports;
        vkViewports.reserve(viewports.size());

        for (uint32_t i = 0; i < viewports.size(); i++)
        {
            vkViewports[i].x        = viewports[i].minX;
            vkViewports[i].y        = viewports[i].minY;
            vkViewports[i].width    = viewports[i].minX + viewports[i].maxX;
            vkViewports[i].height   = viewports[i].minY + viewports[i].maxY;
            vkViewports[i].minDepth = viewports[i].minZ;
            vkViewports[i].maxDepth = viewports[i].maxZ;
        }

        vkCmdSetViewport(m_handle, 0, static_cast<uint32_t>(viewports.size()), vkViewports.data());
    }

    void CommandList::SetScissors(Span<const Rect> scissors)
    {
        std::vector<VkRect2D> vkScissors;
        vkScissors.reserve(scissors.size());

        for (uint32_t i = 0; i < scissors.size(); i++)
        {
            vkScissors[i].extent.width  = scissors[i].sizeX;
            vkScissors[i].extent.height = scissors[i].sizeY;
            vkScissors[i].offset.x      = scissors[i].x;
            vkScissors[i].offset.y      = scissors[i].y;
        }

        vkCmdSetScissor(m_handle, 0, static_cast<uint32_t>(scissors.size()), vkScissors.data());
    }

    void CommandList::Submit(const DrawCommand& drawCmd)
    {
        const PipelineState* pso = static_cast<const PipelineState*>(drawCmd.pPipelineState);
        vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pso->GetHandle());

        if (!drawCmd.descriptorSetPtrs.empty())
        {
            std::vector<VkDescriptorSet> vkDescriptorSets;
            vkDescriptorSets.reserve(drawCmd.descriptorSetPtrs.size());

            const PipelineLayout* pipelineLayout = static_cast<const PipelineLayout*>(drawCmd.pPipelineLayout);

            for (uint32_t i = 0; i < drawCmd.descriptorSetPtrs.size(); i++)
            {
                vkDescriptorSets[i] = static_cast<const DescriptorSet*>(drawCmd.descriptorSetPtrs[i])->GetHandle();
            }

            vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetHandle(), 0,
                                    static_cast<uint32_t>(drawCmd.descriptorSetPtrs.size()), vkDescriptorSets.data(), 0, nullptr);
        }

        Buffer*      pVertexBuffer         = static_cast<Buffer*>(drawCmd.pVertexBuffer);
        VkBuffer     vertexBufferHandle    = pVertexBuffer->GetHandle();
        VkDeviceSize vertexBufferOffset[1] = {0};
        vkCmdBindVertexBuffers(m_handle, 0, 1, &vertexBufferHandle, vertexBufferOffset);

        switch (drawCmd.type)
        {
        case DrawCommand::EDrawType::Linear:
            vkCmdDraw(m_handle, drawCmd.linearDrawDesc.vertexCount, drawCmd.linearDrawDesc.instanceCount, drawCmd.linearDrawDesc.vertexOffset,
                      drawCmd.linearDrawDesc.instanceOffset);
            break;

        case DrawCommand::EDrawType::Indexed:
        {
            Buffer*      pIndexBuffer         = static_cast<Buffer*>(drawCmd.pIndexBuffer);
            VkBuffer     indexBufferHandle    = pIndexBuffer->GetHandle();
            VkDeviceSize indexBufferOffset[1] = {0};
            vkCmdBindIndexBuffer(m_handle, indexBufferHandle, indexBufferOffset[0], VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(m_handle, drawCmd.indexedDrawDesc.indexCount, drawCmd.indexedDrawDesc.instanceCount, drawCmd.indexedDrawDesc.indexOffset,
                             drawCmd.indexedDrawDesc.vertexOffset, drawCmd.indexedDrawDesc.instanceOffset);
        }
        break;
        }
    }

    void CommandList::Submit(const CopyCommand& copyCommand)
    {
        // Buffer to buffer copy
        if (copyCommand.sourceResourceType == RHI::EResourceType::Buffer && copyCommand.destinationResourceType == RHI::EResourceType::Buffer)
        {
            Buffer& sourceBuffer      = static_cast<Buffer&>(*copyCommand.destinationBuffer.pBuffer);
            Buffer& destinationBuffer = static_cast<Buffer&>(*copyCommand.sourceBuffer.pBuffer);

            VkBufferCopy copyRegion = {};
            copyRegion.srcOffset    = copyCommand.sourceBuffer.offset;
            copyRegion.dstOffset    = copyCommand.destinationBuffer.offset;
            copyRegion.size         = copyCommand.sourceBuffer.offset + copyCommand.sourceBuffer.range;
            vkCmdCopyBuffer(m_handle, sourceBuffer.GetHandle(), destinationBuffer.GetHandle(), 1, &copyRegion);
        }
        // Image to Image copy
        else if (copyCommand.sourceResourceType == RHI::EResourceType::Image && copyCommand.destinationResourceType == RHI::EResourceType::Image) {}
        // Buffer to Image Copy
        else if (copyCommand.sourceResourceType == RHI::EResourceType::Buffer && copyCommand.destinationResourceType == RHI::EResourceType::Image) {}
        // Image to Buffer Copy
        else if (copyCommand.sourceResourceType == RHI::EResourceType::Image && copyCommand.destinationResourceType == RHI::EResourceType::Buffer) {}
        else
        {
            assert(false);
        }
    }

    void CommandList::Submit(const DispatchCommand& drawCommand) {}

    void CommandList::BeginConditionalRendering(ConditionBuffer condition)
    {
        VkConditionalRenderingBeginInfoEXT conditionalRenderingBegin;
        vkCmdBeginConditionalRenderingEXT(m_handle, &conditionalRenderingBegin);
    }

    void CommandList::EndConditionalRendering()
    {
        vkCmdEndConditionalRenderingEXT(m_handle);
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(m_pDevice->GetHandle(), m_handle, nullptr);
    }

    VkResult CommandPool::Init(uint32_t queueFamilyIndex)
    {
        VkCommandPoolCreateInfo createInfo;
        createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.queueFamilyIndex = queueFamilyIndex;
        return vkCreateCommandPool(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

    void CommandPool::ResetPool(VkCommandPoolResetFlags flags)
    {
        vkResetCommandPool(m_pDevice->GetHandle(), m_handle, flags);
    }

    std::vector<CommandListPtr> CommandPool::Allocate(Level level, uint32_t count) 
	{
		std::vector<Unique<CommandList>> commandLists(count, CreateUnique<CommandList>(*m_pDevice));
		
		for(auto& cmdList : commandLists)
		{
		}

		return { commandLists.begin(), commandLists.end() };
	}

} // namespace Vulkan
} // namespace RHI
