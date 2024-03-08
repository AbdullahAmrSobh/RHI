#pragma once
#include "StagingBuffer.hpp"
#include "Context.hpp"

namespace RHI::Vulkan
{
    IStagingBuffer::IStagingBuffer(IContext* context)
        : m_context(context)
    {
    }

    IStagingBuffer::~IStagingBuffer()
    {
    }

    VkResult IStagingBuffer::Init()
    {
        return VK_SUCCESS;
    }

    TempBuffer IStagingBuffer::Allocate(size_t newSize)
    {
        BufferCreateInfo createInfo = {};
        createInfo.usageFlags |= BufferUsage::CopySrc;
        createInfo.usageFlags |= BufferUsage::CopyDst;
        createInfo.byteSize = newSize;
        auto buffer = m_context->CreateBuffer(createInfo).GetValue();

        TempBuffer tmpBuffer{};
        tmpBuffer.buffer = buffer;
        tmpBuffer.size = newSize;
        tmpBuffer.offset = 0;
        tmpBuffer.pData = m_context->MapBuffer(tmpBuffer.buffer);
        return tmpBuffer;
    }

    void IStagingBuffer::Free(TempBuffer mappedBuffer)
    {
        m_context->UnmapBuffer(mappedBuffer.buffer);
    }

    void IStagingBuffer::Flush()
    {
        for (auto buffer : m_tempBuffers)
            m_context->DestroyBuffer(buffer.buffer);
        m_tempBuffers.clear();
    }

} // namespace RHI::Vulkan
