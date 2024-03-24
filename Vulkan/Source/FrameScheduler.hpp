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

        void PassSubmit(Pass* pass, Fence* fence) override;
        void StageImageWrite(const BufferToImageCopyInfo& copyInfo) override;

        uint32_t GetCurrentFrameIndex() const { return m_currentFrameIndex; }

    private:
        VkQueue m_graphicsQueue;
        VkQueue m_computeQueue;
        VkQueue m_transferQueue;

        uint32_t m_tmpSemaphoreHead = 0;
        TL::Vector<VkSemaphore> m_tmpSemaphores;
    };
} // namespace RHI::Vulkan