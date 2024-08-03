#include "Examples-Base/Window.hpp"
#include "Examples-Base/Scene.hpp"
#include "Examples-Base/Renderer.hpp"

#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include <TL/Log.hpp>

#include "dds_image/dds.hpp"

namespace Examples
{
    Renderer::Renderer()
    {
    }

    Renderer::~Renderer()
    {
    }

    ResultCode Renderer::Init(const Window& window)
    {
        m_window = &window;

        RHI::ApplicationInfo appInfo{};
        appInfo.applicationName = "Example APP";
        appInfo.applicationVersion.minor = 1;
        appInfo.engineName = "Engine name";
        appInfo.engineVersion.minor = 1;
        m_context = RHI::CreateVulkanContext(appInfo);

        auto windowSize = window.GetWindowSize();
        RHI::SwapchainCreateInfo swapchainCI{};
        swapchainCI.name = "Swapchain";
        swapchainCI.minImageCount = 3;
        swapchainCI.imageSize = { windowSize.width, windowSize.height };
        swapchainCI.imageFormat = RHI::Format::RGBA8_UNORM;
        swapchainCI.imageUsage = RHI::ImageUsage::Color;
        swapchainCI.presentMode = RHI::SwapchainPresentMode::Fifo;
        swapchainCI.win32Window.hwnd = window.GetNativeHandle();
        m_swapchain = m_context->CreateSwapchain(swapchainCI);

        for (auto& cmdPool : m_commandPool)
            cmdPool = m_context->CreateCommandPool(RHI::CommandPoolFlags::Reset);

        for (auto& fence : m_frameFence)
            fence = m_context->CreateFence();

        return OnInit();
    }

    void Renderer::Shutdown()
    {
        OnShutdown(); // must be called first thing here

        for (auto& fence : m_frameFence)
        {
            fence->Wait(UINT64_MAX);
            delete fence.release();
        }

        for (auto& cmdPool : m_commandPool)
        {
            delete cmdPool.release();
        }

        delete m_swapchain.release();
        delete m_context.release();
    }

    void Renderer::Render(const Scene& scene)
    {
        [[maybe_unused]] RHI::ResultCode result;

        {
            static RHI::ImageSize2D size = { m_window->GetWindowSize().width, m_window->GetWindowSize().height };

            auto windowSize = m_window->GetWindowSize();
            if (size.width != windowSize.width || size.height != windowSize.height)
            {
                result = m_swapchain->Recreate(size);
                TL_ASSERT(RHI::IsSucess(result) && "Failed to recreate swapchain on resize");
            }
        }

        OnRender(scene);

        result = m_swapchain->Present();
    }

    Handle<RHI::Image> Renderer::CreateImage(const char* filePath)
    {
        dds::Image image{};
        auto result = dds::readFile(filePath, &image);
        TL_ASSERT(result == dds::Success);

        RHI::ImageCreateInfo createInfo{};
        createInfo.size = { image.width, image.height, image.depth };
        switch (image.format)
        {
        case DXGI_FORMAT_UNKNOWN:
        case DXGI_FORMAT_BC1_UNORM:      createInfo.format = RHI::Format::BC1_UNORM; break;
        case DXGI_FORMAT_BC1_UNORM_SRGB: createInfo.format = RHI::Format::BC1_UNORM_SRGB; break;
        case DXGI_FORMAT_BC2_UNORM:      createInfo.format = RHI::Format::BC2_UNORM; break;
        case DXGI_FORMAT_BC2_UNORM_SRGB: createInfo.format = RHI::Format::BC2_UNORM_SRGB; break;
        case DXGI_FORMAT_BC3_UNORM:      createInfo.format = RHI::Format::BC3_UNORM; break;
        case DXGI_FORMAT_BC3_UNORM_SRGB: createInfo.format = RHI::Format::BC3_UNORM_SRGB; break;
        case DXGI_FORMAT_BC4_UNORM:      createInfo.format = RHI::Format::BC4_UNORM; break;
        case DXGI_FORMAT_BC4_SNORM:      createInfo.format = RHI::Format::BC4_SNORM; break;
        case DXGI_FORMAT_BC5_UNORM:      createInfo.format = RHI::Format::BC5_UNORM; break;
        case DXGI_FORMAT_BC5_SNORM:      createInfo.format = RHI::Format::BC5_SNORM; break;
        case DXGI_FORMAT_B5G6R5_UNORM:   createInfo.format = RHI::Format::B5G6R5_UNORM; break;
        case DXGI_FORMAT_B5G5R5A1_UNORM: createInfo.format = RHI::Format::B5G5R5A1_UNORM; break;
        default:                         createInfo.format = RHI::Format::Unknown; break;
        };

        createInfo.arrayCount = image.arraySize;
        createInfo.mipLevels = (uint32_t)image.mipmaps.size();
        createInfo.usageFlags = RHI::ImageUsage::ShaderResource | RHI::ImageUsage::CopyDst;
        createInfo.sampleCount = RHI::SampleCount::Samples1;
        createInfo.type = image.dimension == dds::ResourceDimension::Texture1D ? RHI::ImageType::Image1D : (image.dimension == dds::ResourceDimension::Texture2D ? RHI::ImageType::Image2D : RHI::ImageType::Image3D);
        createInfo.name = filePath;
        return CreateImageWithData(createInfo, TL2::Span<const uint8_t>{ image.data.data(), image.data.size() }).GetValue();
    }

   TL::Ptr<Scene> Renderer::CreateScene()
    {
        return TL::CreatePtr<Scene>(m_context.get());
    }
} // namespace Examples