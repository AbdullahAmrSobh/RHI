#include "RHI/Context.hpp"
#include "RHI/FrameScheduler.hpp"

#include <tracy/Tracy.hpp>

namespace RHI
{
    Result<Handle<Image>> Context::CreateImage(const ImageCreateInfo& createInfo, TL::Span<const uint8_t> content)
    {
        ZoneScoped;
        auto [handle, result] = CreateImage(createInfo);
        if (IsError(result))
            return result;
        m_frameScheduler->WriteImageContent(handle, {}, createInfo.size, {}, content);
        return handle;
    }

    Result<Handle<Buffer>> Context::CreateBuffer(const BufferCreateInfo& createInfo, TL::Span<const uint8_t> content)
    {
        ZoneScoped;
        auto [handle, result] = CreateBuffer(createInfo);
        if (IsError(result))
            return result;
        if (m_limits->stagingMemoryLimit >= createInfo.byteSize)
        {
            auto ptr = MapBuffer(handle);
            memcpy(ptr, content.data(), content.size());
            UnmapBuffer(handle);
        }
        else
        {
            RHI_UNREACHABLE();
        }
        return handle;
    }

    void Context::OnDestruct()
    {
        ZoneScoped;

        m_frameScheduler->Cleanup();

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

        m_frameScheduler.reset();
        m_limits.reset();
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
} // namespace RHI