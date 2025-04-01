#include "StagingBuffer.hpp"

#include <tracy/Tracy.hpp>

#include "Device.hpp"

namespace RHI::Vulkan
{
    constexpr static size_t MinStagingBufferAllocationSize = 64 * 1024 * 1024; // 64 mb

    StagingBuffer::StagingBuffer()  = default;
    StagingBuffer::~StagingBuffer() = default;

    ResultCode StagingBuffer::Init(IDevice* device)
    {
        m_device = device;
        return ResultCode::Success;
    }

    void StagingBuffer::Shutdown()
    {
        for (auto& page : m_pages)
        {
            // m_device->UnmapBuffer(page.buffer);
            m_device->DestroyBuffer(page.buffer);
        }
    }

    StagingBlock StagingBuffer::Allocate(size_t size)
    {
        for (auto& page : m_pages)
        {
            if (page.GetRemainingSize() >= size)
            {
                auto         offset       = page.offset;
                StagingBlock stagingBlock = {page.buffer, offset};
                page.offset += size;
                return stagingBlock;
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

        auto bufferHandle = m_device->CreateBuffer(stagingBufferCI).GetValue();
        auto buffer       = m_device->m_bufferOwner.Get(bufferHandle);

        m_pages.push_back(
            Page{
                .ptr    = buffer->Map(m_device),
                .buffer = bufferHandle,
                .offset = size,
                .size   = stagingBufferCI.byteSize,
            });

        return StagingBlock{
            .buffer = m_pages.back().buffer,
            .offset = 0,
        };
    }

    StagingBlock StagingBuffer::Allocate(TL::Block block)
    {
        auto stagingBlock = Allocate(block.size);
        auto buffer       = m_device->m_bufferOwner.Get(stagingBlock.buffer);
        auto ptr          = (char*)buffer->Map(m_device) + stagingBlock.offset;
        memcpy(ptr, block.ptr, block.size);
        return stagingBlock;
    }

    void StagingBuffer::ReleaseAll()
    {
        for (auto& page : m_pages)
        {
            page.offset = 0;
        }
    }

} // namespace RHI::Vulkan