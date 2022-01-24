#pragma once
#include "RHI/Backend/Vulkan/Fence.hpp"
#include "RHI/Backend/Vulkan/RenderTarget.hpp"
#include "RHI/Commands.hpp"

#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Texture.hpp"

namespace RHI
{
namespace Vulkan
{

    class CommandList final
        : public ICommandList
        , public DeviceObject<VkCommandBuffer>
    {
        virtual void Reset() override;
        virtual void Begin() override;
        virtual void End() override;

        virtual void SetViewports(const Viewport* viewports, uint32_t count) override;
        virtual void SetScissors(const Rect* scissors, uint32_t count) override;

        // virtual void SetClearColor(const Color& color) override;

        virtual void Submit(const CopyCommand& command) override;
        virtual void Submit(const DrawCommand& command) override;
        virtual void Submit(const DispatchCommand& command) override;

    private:
        inline void WaitForResourceIsReady(const Texture& texture, VkPipelineStageFlags stage)
        {
            m_waitPoint.push_back({texture.GetResourceIsReadySemaphore(), stage});
        }

    public:
        struct WaitPoint
        {
            VkSemaphore          semaphore;
            VkPipelineStageFlags stage;
        };

        inline ArrayView<WaitPoint> GetWaitPoints() const { return ArrayView<WaitPoint>(m_waitPoint.begin()._Ptr, m_waitPoint.end()._Ptr); }
        
        inline ArrayView<VkSemaphore> GetSignalSemaphores() const
        {
            return ArrayView<VkSemaphore>(m_signalSemaphores.begin()._Ptr, m_signalSemaphores.end()._Ptr);
        }

    private:
        std::vector<WaitPoint>   m_waitPoint;
        std::vector<VkSemaphore> m_signalSemaphores;
    };

    class SubmitInfo
    {
    public:
        SubmitInfo(ArrayView<ICommandList*> cmdLists)
        {
            std::for_each(cmdLists.begin(), cmdLists.end(),
                          [&](ICommandList* _pCommandList)
                          {
                              CommandList* commandList = static_cast<CommandList*>(_pCommandList);

                              auto waitPoints = commandList->GetWaitPoints();

                              std::for_each(waitPoints.begin(), waitPoints.end(),
                                            [&](const CommandList::WaitPoint& waitSemaphore)
                                            {
                                                m_waitSemaphores.push_back(waitSemaphore.semaphore);
                                                m_dstStageMasks.push_back(waitSemaphore.stage);
                                            });

                              std::transform(commandList->GetSignalSemaphores().begin(), commandList->GetSignalSemaphores().end(),
                                             std::back_inserter(m_signalSemaphores), [](VkSemaphore semaphore) { return semaphore; });

                              m_commandBuffers.push_back(commandList->GetHandle());
                          });

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
        friend class CommandQueue;
        VkSubmitInfo                      m_submitInfo;
        std::vector<VkSemaphore>          m_waitSemaphores;
        std::vector<VkPipelineStageFlags> m_dstStageMasks;
        std::vector<VkCommandBuffer>      m_commandBuffers;
        std::vector<VkSemaphore>          m_signalSemaphores;
    };

    class CommandQueue final
        : public ICommandQueue
        , public DeviceObject<VkQueue>
    {
    public:
        CommandQueue(Device& device, uint32_t queueFamilyIndex, uint32_t queueIndex)
            : DeviceObject(device)
            , m_queueFamilyIndex(queueFamilyIndex)
            , m_queueIndex(queueIndex)
        {
            vkGetDeviceQueue(m_pDevice->GetHandle(), m_queueFamilyIndex, m_queueIndex, &m_handle);
        }
        
        virtual void Submit(ArrayView<ICommandList*> commandLists, IFence& signalFence) override
        {
            SubmitInfo   submitInfo(commandLists);
            VkFence      fenceToSignal = static_cast<Fence*>(&signalFence)->GetHandle();
            VkSubmitInfo submitInfos[] = {submitInfo.m_submitInfo};
            Assert(vkQueueSubmit(m_handle, 1, submitInfos, fenceToSignal));
        }
    
    private:
        uint32_t m_queueFamilyIndex;
        uint32_t m_queueIndex;
    };

} // namespace Vulkan
} // namespace RHI
