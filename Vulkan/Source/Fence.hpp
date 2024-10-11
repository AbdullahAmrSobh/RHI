#pragma once

#include <RHI/Result.hpp>
#include <RHI/Fence.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;

    class IFence final : public Fence
    {
    public:
        IFence(IDevice* device)
            : m_device(device)
        {
        }

        ~IFence();

        ResultCode Init();

        void Reset() override;
        bool WaitInternal(uint64_t timeout) override;
        FenceState GetState() override;

        // This should only be called when passing the fence to a vulkan siganl command
        VkFence UseFence();

    private:
        IDevice* m_device;
        VkFence m_fence;
        FenceState m_state;
    };
} // namespace RHI
