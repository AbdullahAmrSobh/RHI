#include "RHI/Context.hpp"
#include "RHI/Resources.hpp"

#include "RHI/Common/Span.hpp"

namespace RHI
{
    template<typename T>
    inline static Result<Handle<Image>> CreateImageWithData(Context& context, const ImageCreateInfo& createInfo, TL::Span<const T> content)
    {
        auto [handle, result] = context.CreateImage(createInfo);

        if (result != ResultCode::Success)
            return result;

        auto stagingBuffer = context.AllocateTempBuffer(content.size_bytes());
        memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());
        context.StageResourceWrite(handle, {}, stagingBuffer.buffer, stagingBuffer.offset);

        return handle;
    }

    template<typename T>
    inline static Result<Handle<Buffer>> CreateBufferWithData(Context& context, Flags<BufferUsage> usageFlags, TL::Span<const T> content)
    {
        BufferCreateInfo createInfo{};
        createInfo.byteSize   = content.size_bytes();
        createInfo.usageFlags = usageFlags;

        auto [handle, result] = context.CreateBuffer(createInfo);

        if (result != ResultCode::Success)
            return result;

        if (content.size_bytes() <= context.GetLimits().stagingMemoryLimit)
        {
            auto ptr = context.MapBuffer(handle);
            memcpy(ptr, content.data(), content.size_bytes());
            context.UnmapBuffer(handle);
        }
        else
        {
            auto stagingBuffer = context.AllocateTempBuffer(content.size_bytes());
            memcpy(stagingBuffer.ptr, content.data(), content.size_bytes());
            context.StageResourceWrite(handle, 0, content.size_bytes(), stagingBuffer.buffer, stagingBuffer.offset);
        }

        return handle;
    }
} // namespace RHI