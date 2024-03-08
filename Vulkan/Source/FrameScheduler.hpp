#pragma once

#include <RHI/FrameScheduler.hpp>

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

    private:
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;

        std::vector<Handle<Image>> m_stagedWriteImages;
        std::vector<Handle<Buffer>> m_stagedWriteBuffers;
        std::vector<Handle<Image>> m_stagedReadImages;
        std::vector<Handle<Buffer>> m_stagedReadBuffers;

        uint32_t m_tmpSemaphoreHead = 0;
        std::vector<VkSemaphore> m_tmpSemaphores;
    };
} // namespace RHI::Vulkan