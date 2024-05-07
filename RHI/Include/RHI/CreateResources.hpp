#include "RHI/Context.hpp"
#include "RHI/Resources.hpp"
#include "RHI/FrameScheduler.hpp"

#include "RHI/Common/Span.hpp"

namespace RHI
{
    template<typename T>
    inline static Result<Handle<Image>> CreateImageWithData(Context& context, const ImageCreateInfo& createInfo, TL::Span<const T> content)
    {
        auto& scheduler       = context.GetScheduler();

        auto [handle, result] = context.CreateImage(createInfo);

        if (result != ResultCode::Success)
            return result;

        RHI::BufferCreateInfo _createInfo{};
        _createInfo.byteSize            = content.size_bytes();
        _createInfo.usageFlags          = BufferUsage::CopySrc;
        auto                  tmpBuffer = context.CreateBuffer(_createInfo).GetValue();

        BufferToImageCopyInfo copyInfo{};
        copyInfo.srcBuffer = tmpBuffer;
        copyInfo.srcOffset = 0;
        copyInfo.dstImage  = handle;
        auto ptr           = context.MapBuffer(tmpBuffer);
        memcpy(ptr, content.data(), content.size_bytes());
        context.UnmapBuffer(tmpBuffer);

        scheduler.WriteImageContent(handle, {}, createInfo.size, {}, content);

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
            RHI_UNREACHABLE();
        }

        return handle;
    }
} // namespace RHI