#include "StagingBuffer.hpp"
#include "Device.hpp"

#include <tracy/Tracy.hpp>

namespace RHI::Vulkan
{

    void StagingBufferAllocator::Shutdown()
    {
        for (auto& page : m_pages)
        {
            m_device->UnmapBuffer(page.buffer);
            m_device->DestroyBuffer(page.buffer);
        }
    }

    StagingBuffer StagingBufferAllocator::Allocate(size_t size)
    {
        ZoneScoped;

        for (auto& page : m_pages)
        {
            if (page.GetRemainingSize() >= size)
            {
                StagingBuffer allocation{
                    .ptr    = page.ptr,
                    .buffer = page.buffer,
                    .offset = page.offset,
                };

                page.offset += size;

                return allocation;
            }
        }

        std::string      name = std::format("StagingBuffer-{}", m_pages.size());
        BufferCreateInfo stagingBufferCI{
            .name       = name.c_str(),
            .heapType   = MemoryType::GPUShared,
            .usageFlags = BufferUsage::CopyDst | BufferUsage::CopySrc,
            .byteSize   = std::max(size, (size_t)6.4e+7),
        };

        auto buffer = m_device->CreateBuffer(stagingBufferCI).GetValue();
        m_pages.push_back({.ptr = m_device->MapBuffer(buffer), .buffer = buffer, .offset = size, .size = stagingBufferCI.byteSize});
        return StagingBuffer{
            .ptr    = m_pages.back().ptr,
            .buffer = m_pages.back().buffer,
            .offset = 0,
        };
    }

    void StagingBufferAllocator::ReleaseAll()
    {
        for (auto& page : m_pages)
        {
            page.offset = 0;
        }
    }

} // namespace RHI::Vulkan