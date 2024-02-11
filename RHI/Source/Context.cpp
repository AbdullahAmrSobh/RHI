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
        
        auto tempBuffer = m_streamingBuffer->Allocate(content.size());
        memcpy(tempBuffer.pData, content.data(), content.size());
        m_streamingBuffer->Free(tempBuffer);

        RHI_ASSERT(CalculateRequiredSize(createInfo) == content.size());

        auto cmdList = m_copyCommandsAllocator->Allocate();
        cmdList->Begin();
        m_frameScheduler->StreamResource(*cmdList, handle, {}, createInfo.size, tempBuffer);
        cmdList->End();

        auto fence = CreateFence();
        GetScheduler()->QueueCommandsSubmit(QueueType::Transfer, cmdList, *fence);
        fence->Wait();

        return handle;
    }

    Result<Handle<Buffer>> Context::CreateBufferWithContent(const BufferCreateInfo& createInfo, TL::Span<uint8_t> content)
    {
        auto [handle, result] = CreateBuffer(createInfo);

        if (IsError(result))
        {
            return result;
        }

        auto tempBuffer = m_streamingBuffer->Allocate(content.size());
        memcpy(tempBuffer.pData, content.data(), content.size());
        m_streamingBuffer->Free(tempBuffer);

        RHI_ASSERT(CalculateRequiredSize(createInfo) == content.size());
        
        auto cmdList = m_copyCommandsAllocator->Allocate();
        cmdList->Begin();
        m_frameScheduler->StreamResource(*cmdList, handle, {}, createInfo.byteSize, tempBuffer);
        cmdList->End();
        
        auto fence = CreateFence();
        GetScheduler()->QueueCommandsSubmit(QueueType::Transfer, cmdList, *fence);
        fence->Wait();

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