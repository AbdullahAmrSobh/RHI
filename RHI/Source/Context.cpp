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

        auto ptr = MapBuffer(handle);
        memcpy(ptr, content.data(), content.size());
        UnmapBuffer(handle);

        return handle;
    }

    void Context::DebugLogError(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogError(message);
#else
        (void)message;
#endif
    }

    void Context::DebugLogWarn(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogWarnning(message);
#else
        (void)message;
#endif
    }

    void Context::DebugLogInfo(std::string_view message)
    {
        ZoneScoped;

#if RHI_DEBUG
        if (m_debugCallbacks)
            m_debugCallbacks->LogInfo(message);
#else
        (void)message;
#endif
    }

    void Context::DebugReportLiveResources()
    {
        ZoneScoped;

#if RHI_DEBUG
        if (auto count = m_LeakDetector.m_images.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_images.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_buffers.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_buffers.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_imageViews.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_imageViews.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_bufferViews.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_bufferViews.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_bindGroupLayouts.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_bindGroupLayouts.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_bindGroups.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_bindGroups.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_pipelineLayouts.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_pipelineLayouts.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_graphicsPipelines.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_graphicsPipelines.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_computePipelines.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_computePipelines.ReportLiveResources());
        }

        if (auto count = m_LeakDetector.m_samplers.LeakedResourcesCount(); count != 0)
        {
            DebugLogInfo(m_LeakDetector.m_samplers.ReportLiveResources());
        }
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