#pragma once
#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Result.hpp"
#include "RHI/Resources.hpp"

#include <TL/Flags.hpp>
#include <TL/Span.hpp>
#include <TL/UniquePtr.hpp>

namespace RHI
{
    struct BufferRange
    {
        Handle<Buffer>  buffer;
        BufferSubregion subregion;
        uint32_t        _metaData;
    };

    // Buffer pool is a buffer that can you suballocate ranges from
    class RHI_EXPORT BufferPool
    {
    public:
        BufferPool() = default;
        BufferPool(Handle<Buffer> buffer, size_t bufferSize);

        BufferPool(const BufferPool&)            = delete;
        BufferPool& operator=(const BufferPool&) = delete;

        BufferPool(BufferPool&&)            = default;
        BufferPool& operator=(BufferPool&&) = default;

        ~BufferPool();

        static BufferPool   Create(Context* context, const BufferCreateInfo& createInfo);
        Result<BufferRange> Allocate(size_t size, size_t alignment);
        void                Release(BufferRange buffer);

    private:
        class Impl;
        Context*                  m_context;
        TL::Ptr<Impl> m_impl;
        Handle<Buffer>            m_buffer;
    };
} // namespace RHI