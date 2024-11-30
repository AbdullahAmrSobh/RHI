#pragma once

#include <RHI/Queue.hpp>
#include <RHI/Result.hpp>

#include <vulkan/vulkan.h>

#include <TL/Span.hpp>

namespace RHI::Vulkan
{
    class IDevice;
    class ICommandList;

    class IQueue
    {
    public:
        IQueue();
        ~IQueue();

        ResultCode Init(IDevice* device, uint32_t familyIndex, uint32_t queueIndex);
        void       Shutdown();

        inline VkQueue GetHandle() const { return m_queue; }

        inline uint32_t GetFamilyIndex() const { return m_familyIndex; }

        void BeginLabel(const char* name, float color[4]);

        void EndLabel();

        uint64_t Submit(const SubmitInfo& submitInfo);

    private:
        IDevice* m_device;
        VkQueue  m_queue;
        uint32_t m_familyIndex;
    };
} // namespace RHI::Vulkan