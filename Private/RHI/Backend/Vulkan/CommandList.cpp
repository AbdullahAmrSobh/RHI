#include "RHI/Backend/Vulkan/CommandList.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"

#include "RHI/Backend/Vulkan/DescriptorSet.hpp"
#include "RHI/Backend/Vulkan/PipelineLayout.hpp"
#include "RHI/Backend/Vulkan/PipelineState.hpp"

namespace RHI
{
namespace Vulkan
{
    CommandContext::~CommandContext() {}

    void CommandContext::Begin()
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags                    = 0;
        beginInfo.pNext                    = nullptr;
        beginInfo.pInheritanceInfo         = nullptr;
        vkBeginCommandBuffer(m_handle, &beginInfo);
    }

    void CommandContext::End() { vkEndCommandBuffer(m_handle); }

    void CommandContext::SetViewports(ArrayView<const Viewport> viewports)
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

    void CommandContext::SetScissors(ArrayView<const Rect> scissors)
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

    void CommandContext::Submit(const DrawCommand& drawCmd)
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
			Buffer* pIndexBuffer = static_cast<Buffer*>(drawCmd.pIndexBuffer);
			VkBuffer indexBufferHandle = pIndexBuffer->GetHandle();
			VkDeviceSize indexBufferOffset[1] = {0};
			vkCmdBindIndexBuffer(m_handle, indexBufferHandle, indexBufferOffset[0], VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(m_handle, drawCmd.indexedDrawDesc.indexCount, drawCmd.indexedDrawDesc.instanceCount, drawCmd.indexedDrawDesc.indexOffset,
				drawCmd.indexedDrawDesc.vertexOffset, drawCmd.indexedDrawDesc.instanceOffset);
		} break;
        }
    
    }

} // namespace Vulkan
} // namespace RHI
