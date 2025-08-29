#include <RHI/RHI.hpp>
#include <RHI/internal/renderdoc_app.h>

#include <TL/Assert.hpp>
#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
    Image* CreateImageWithContent(Device& device, const ImageCreateInfo& createInfo, TL::Block content)
    {
        // auto imageSizeBytes = CalcaulteImageSize(createInfo.format, createInfo.size, createInfo.mipLevels, createInfo.arrayCount);
        auto imageSizeBytes = content.size;

        TL_ASSERT(imageSizeBytes != 0);
        // TL_ASSERT(imageSizeBytes == content.size);
        TL_ASSERT(createInfo.usageFlags & ImageUsage::CopyDst);

        auto image = device.CreateImage(createInfo);
        auto frame = device.GetCurrentFrame();
        frame->ImageWrite(image, ImageOffset3D{}, createInfo.size, 0, 0, content);
        return image;
    }

    DeviceLimits Device::GetLimits() const
    {
        return *m_limits;
    }

    RenderGraph* Device::CreateRenderGraph([[maybe_unused]] const RenderGraphCreateInfo& createInfo)
    {
        ZoneScoped;
        auto* renderGraph = new RenderGraph();
        auto  result      = renderGraph->Init(this, createInfo);
        TL_ASSERT(result == ResultCode::Success);
        return renderGraph;
    }

    void Device::DestroyRenderGraph(RenderGraph* renderGraph)
    {
        ZoneScoped;
        TL_ASSERT(renderGraph != nullptr, "Cannot destroy a null render graph");
        renderGraph->Shutdown();
        delete renderGraph;
    }

    ResultCode Renderdoc::Init(Device* device)
    {
        m_device = device;

        auto [library, result] = TL::Library::Open("renderdoc.dll");
        if (result.IsError())
        {
            TL_LOG_ERROR("Failed to load RenderDoc API: {}", result.GetMessage());
            return ResultCode::ErrorUnknown;
        }

        m_library    = library;
        auto* getAPI = reinterpret_cast<pRENDERDOC_GetAPI>(m_library.GetProc("RENDERDOC_GetAPI"));
        if (!getAPI)
        {
            TL_LOG_ERROR("Failed to get RENDERDOC_GetAPI function pointer");
            TL::Library::Close(m_library);
            return ResultCode::ErrorUnknown;
        }

        auto api = reinterpret_cast<RENDERDOC_API_1_6_0**>(&m_renderdocAPi);
        if (!getAPI(eRENDERDOC_API_Version_1_6_0, reinterpret_cast<void**>(api)))
        {
            TL_LOG_ERROR("Failed to initialize RenderDoc API");
            TL::Library::Close(m_library);
            return ResultCode::ErrorUnknown;
        }

        if (api)
            (*api)->MaskOverlayBits(0, 0);

        TL_LOG_INFO("Renderdoc connected");
        return ResultCode::Success;
    }

    void Renderdoc::Shutdown()
    {
        if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
            api->Shutdown();
        TL::Library::Close(m_library);
    }

    void Renderdoc::FrameTriggerMultiCapture(uint32_t numFrames)
    {
        if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
            api->TriggerMultiFrameCapture(numFrames);
    }

    bool Renderdoc::FrameIsCapturing()
    {
        if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
            return api->IsFrameCapturing();
        return false;
    }

    void Renderdoc::FrameStartCapture()
    {
        if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
        {
            auto device = (void*)m_device->GetNativeHandle(NativeHandleType::Device, (uint64_t)m_device);
            api->StartFrameCapture(device, nullptr);
        }
    }

    void Renderdoc::FrameEndCapture()
    {
        if (auto* api = (RENDERDOC_API_1_6_0*)m_renderdocAPi)
        {
            auto device = (void*)m_device->GetNativeHandle(NativeHandleType::Device, (uint64_t)m_device);
            api->EndFrameCapture(device, nullptr);
        }
    }

} // namespace RHI