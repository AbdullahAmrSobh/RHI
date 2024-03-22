#pragma once

#include <RHI/FrameScheduler.hpp>

#include <RHI/Common/Containers.h>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    class IFrameScheduler final : public FrameScheduler
    {
    public:
        IFrameScheduler(IContext* context);
        ~IFrameScheduler();

        VkResult Init();

        void Flush() override;

        void WaitIdle() override;
        void PassSubmit(Pass* pass, Fence* fence) override;

        void StageImageWrite(Handle<Image> handle) override;
        void StageBufferWrite(Handle<Buffer> handle) override;

        void ExecuteCommandLists(CommandList& commandList, Fence* fence) override;

        VkSemaphore CreateTempSemaphore();

        uint32_t GetCurrentFrameIndex() const { return m_currentFrameIndex; }

    private:
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;

        TL::Vector<Handle<Image>> m_stagedWriteImages;
        TL::Vector<Handle<Buffer>> m_stagedWriteBuffers;
        TL::Vector<Handle<Image>> m_stagedReadImages;
        TL::Vector<Handle<Buffer>> m_stagedReadBuffers;

        uint32_t m_tmpSemaphoreHead = 0;
        TL::Vector<VkSemaphore> m_tmpSemaphores;
    };
} // namespace RHI::Vulkan