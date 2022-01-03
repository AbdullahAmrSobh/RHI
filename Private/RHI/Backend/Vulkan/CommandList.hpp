#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Texture.hpp"

namespace RHI
{
namespace Vulkan
{

    class Semaphore : public DeviceObject<VkSemaphore>
    {
    public:
        Semaphore(Device& device)
            : DeviceObject(device)
        {
        }

        inline ~Semaphore() { vkDestroySemaphore(m_pDevice->GetHandle(), m_handle, nullptr); }
        inline VkResult Init()
        {
            VkSemaphoreCreateInfo info = {};
            info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            info.pNext                 = nullptr;
            info.flags                 = 0;
            return vkCreateSemaphore(m_pDevice->GetHandle(), &info, nullptr, &m_handle);
        }
    };

    class DrawCommand : DeviceObject<VkCommandBuffer>
    {
    public:
        void DrawIndexedInstanced(Buffer& instanceBuffer, uint32_t instanceCount, Buffer& vertexBuffer, uint32_t vertexCount, Buffer& indexBuffer,
                                  uint32_t indexCount)
        {
            VkCommandBufferBeginInfo beginInfo           = {};
            VkRenderPassBeginInfo    renderPassBeginInfo = {};
            std::vector<VkViewport>  viewports;
            std::vector<VkRect2D>    scissors;

            VkPipeline       pipeline;
            VkPipelineLayout pipelineLayout;

            uint32_t                     firstSet = 0;
            std::vector<VkDescriptorSet> descriptorSets;
            std::vector<uint32_t>        dynamicOffsets;
                        
            VkBuffer     buffers[2] = {instanceBuffer.GetHandle(), vertexBuffer.GetHandle()};
            VkDeviceSize offsets[2] = {0, 0};

            vkBeginCommandBuffer(m_handle, &beginInfo);
            vkCmdBeginRenderPass(m_handle, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdSetViewport(m_handle, 0, viewports.size(), viewports.data());
            vkCmdSetScissor(m_handle, 0, scissors.size(), scissors.data());
            vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, firstSet, descriptorSets.size(), descriptorSets.data(),
                                    dynamicOffsets.size(), dynamicOffsets.data());
            // vkCmdPipelineBarrier(m_handle, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t
            // memoryBarrierCount, const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier
            // *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers)
            vkCmdBindVertexBuffers(m_handle, 0, 2, buffers, offsets);
            vkCmdBindIndexBuffer(m_handle, instanceBuffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDraw(m_handle, vertexCount, instanceCount, 0, 0);
            vkCmdDrawIndexed(m_handle, indexCount, instanceCount, 0, 0, 0);
            vkCmdEndRenderPass(m_handle);
            vkEndCommandBuffer(m_handle);
        }

    private:
    };

    class SubmitInfo
    {
    public:
        SubmitInfo(const std::vector<VkSemaphore>& waitSemaphores, const std::vector<VkPipelineStageFlags>& waitSemaphoreStageMasks,
                   const std::vector<VkCommandBuffer>& commandBuffers, const std::vector<VkSemaphore>& signalSemaphores)
            : m_waitSemaphores(waitSemaphores)
            , m_dstStageMasks(waitSemaphoreStageMasks)
            , m_signalSemaphores(signalSemaphores)
            , m_commandBuffers(commandBuffers)
        {
            m_submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.pNext                = nullptr;
            m_submitInfo.waitSemaphoreCount   = static_cast<uint32_t>(m_waitSemaphores.size());
            m_submitInfo.pWaitSemaphores      = m_waitSemaphores.data();
            m_submitInfo.pWaitDstStageMask    = m_dstStageMasks.data();
            m_submitInfo.commandBufferCount   = static_cast<uint32_t>(m_commandBuffers.size());
            m_submitInfo.pCommandBuffers      = m_commandBuffers.data();
            m_submitInfo.signalSemaphoreCount = static_cast<uint32_t>(m_signalSemaphores.size());
            m_submitInfo.pSignalSemaphores    = m_signalSemaphores.data();
        }

    private:
        VkSubmitInfo                      m_submitInfo;
        std::vector<VkSemaphore>          m_waitSemaphores;
        std::vector<VkPipelineStageFlags> m_dstStageMasks;
        std::vector<VkCommandBuffer>      m_commandBuffers;
        std::vector<VkSemaphore>          m_signalSemaphores;
    };

} // namespace Vulkan
} // namespace RHI
