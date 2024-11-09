#pragma once

#include <RHI/Queue.hpp>

#include <TL/Span.hpp>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IDevice;
    class ICommandList;

    class IQueue final : public Queue
    {
    public:
        IQueue() = default;
        IQueue(IDevice* device, uint32_t familyIndex);

        inline VkQueue GetHandle() const { return m_queue; }

        inline uint32_t GetFamilyIndex() const { return m_familyIndex; }

        void BeginLabel(const char* name, float color[4]) override;

        void EndLabel() override;

        uint64_t Submit(const SubmitInfo& submitInfo) override;

    private:
        IDevice* m_device;
        VkQueue  m_queue;
        uint32_t m_familyIndex;
    };
} // namespace RHI::Vulkan