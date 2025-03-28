#include "StagingBuffer.hpp"

#include <tracy/Tracy.hpp>

#include "Device.hpp"

namespace RHI::Vulkan
{
    StagingBufferAllocator::StagingBufferAllocator()  = default;
    StagingBufferAllocator::~StagingBufferAllocator() = default;

    ResultCode StagingBufferAllocator::Init(IDevice* device)
    {
        m_device = device;
        return ResultCode::Success;
    }

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
            .hostMapped = true,
            .usageFlags = BufferUsage::CopyDst | BufferUsage::CopySrc,
            // .byteSize   = std::max(size, static_cast<size_t>(64 * 1024 * 1024)), // 64 MB
            .byteSize   = size,
        };

        auto buffer = m_device->CreateBuffer(stagingBufferCI).GetValue();
        m_pages.push_back({.ptr    = m_device->MapBuffer(buffer),
                           .buffer = buffer,
                           .offset = size,
                           .size   = stagingBufferCI.byteSize});

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