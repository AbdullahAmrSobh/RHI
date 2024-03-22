#pragma once
#include <RHI/StagingBuffer.hpp>

#include <RHI/Common/Containers.h>

#include <vulkan/vulkan.h>

namespace RHI::Vulkan
{
    class IContext;

    class IStagingBuffer final : public StagingBuffer
    {
    public:
        IStagingBuffer(IContext* context);
        ~IStagingBuffer();

        VkResult Init();

        TempBuffer Allocate(size_t newSize) override;
        void Free(TempBuffer mappedBuffer) override;
        void Flush() override;

    private:
        IContext* m_context;

        TL::Vector<TempBuffer> m_tempBuffers;
    };
} // namespace RHI::Vulkan