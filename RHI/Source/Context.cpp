#include "RHI/Context.hpp"

namespace RHI
{
    Result<Handle<Image>> Context::CreateImageWithContent(const ImageCreateInfo& createInfo, TL::Span<uint8_t> content)
    {
        auto [handle, result] = CreateImage(createInfo);

        if (IsError(result))
        {
            return result;
        }
        (void)content;
        RHI_UNREACHABLE();
        return handle;
    }

    Result<Handle<Buffer>> Context::CreateBufferWithContent(const BufferCreateInfo& createInfo, TL::Span<uint8_t> content)
    {
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
#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogError(message, __VA_ARGS__);
#else
        (void)message;
#endif
    }

    void Context::DebugLogWarn(const char* message, ...)
    {
#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogWarnning(message, __VA_ARGS__);
#else
        (void)message;
#endif
    }

    void Context::DebugLogInfo(const char* message, ...)
    {
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