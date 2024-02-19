#include "RHI/Context.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    Result<Handle<Image>> Context::CreateImageWithContent(const ImageCreateInfo& createInfo, TL::Span<uint8_t> content)
    {
        ZoneScoped;

        auto [handle, result] = CreateImage(createInfo);

        if (IsError(result))
        {
            return result;
        }

        BufferCreateInfo stagingBufferInfo{};
        stagingBufferInfo.byteSize = CalculateRequiredSize(createInfo);
        stagingBufferInfo.usageFlags = RHI::BufferUsage::CopySrc;
        auto stagingBuffer = CreateBuffer(stagingBufferInfo).GetValue();
        auto ptr = MapBuffer(stagingBuffer);
        memcpy(ptr, content.data(), content.size());
        UnmapBuffer(stagingBuffer);

        auto fence = CreateFence();
        auto command = m_transferCommandsAllocator->Allocate();
        BufferToImageCopyInfo copyInfo{};
        copyInfo.srcBuffer = stagingBuffer;
        copyInfo.srcOffset = 0;
        copyInfo.srcSize.width = createInfo.size.width;
        copyInfo.srcSize.height = createInfo.size.height;
        copyInfo.srcSize.depth = createInfo.size.depth;
        copyInfo.dstImage = handle;
        copyInfo.dstSubresource.imageAspects = RHI::ImageAspect::Color;
        command->Begin();
        command->Copy(copyInfo);
        command->End();
        m_frameScheduler->ExecuteCommandList(command, *fence);
        fence->Wait();

        return handle;
    }

    Result<Handle<Buffer>> Context::CreateBufferWithContent(const BufferCreateInfo& createInfo, TL::Span<uint8_t> content)
    {
        ZoneScoped;

        auto [handle, result] = CreateBuffer(createInfo);

        if (IsError(result))
        {
            return result;
        }
        (void)content;
        RHI_UNREACHABLE();
        return handle;
    }

    void Context::DebugLogError(const char* message, ...)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogError(message, __VA_ARGS__);
#else
        (void)message;
#endif
    }

    void Context::DebugLogWarn(const char* message, ...)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogWarnning(message, __VA_ARGS__);
#else
        (void)message;
#endif
    }

    void Context::DebugLogInfo(const char* message, ...)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogInfo(message, __VA_ARGS__);
#else
        (void)message;
#endif
    }

    size_t Context::CalculateRequiredSize(const ImageCreateInfo& createInfo)
    {
        auto formatInfo = GetFormatInfo(createInfo.format);
        return formatInfo.bytesPerBlock * createInfo.size.width * createInfo.size.height * createInfo.size.depth;
    }

    size_t Context::CalculateRequiredSize(const BufferCreateInfo& createInfo)
    {
        return createInfo.byteSize;
    }

} // namespace RHI