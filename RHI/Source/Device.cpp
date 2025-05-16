#include <RHI/RHI.hpp>

#include <TL/Assert.hpp>
#include <TL/Log.hpp>

#include <tracy/Tracy.hpp>

namespace RHI
{
    Handle<Image> CreateImageWithContent(Device& device, const ImageCreateInfo& createInfo, TL::Block content)
    {
        // auto imageSizeBytes = CalcaulteImageSize(createInfo.format, createInfo.size, createInfo.mipLevels, createInfo.arrayCount);
        auto imageSizeBytes = content.size;

        TL_ASSERT(imageSizeBytes != 0);
        // TL_ASSERT(imageSizeBytes == content.size);
        TL_ASSERT(createInfo.usageFlags & ImageUsage::CopyDst);

        auto image = device.CreateImage(createInfo);
        device.ImageWrite(image, ImageOffset3D{}, createInfo.size, 0, 0, content);
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
        renderGraph->Init(this, createInfo);
        return renderGraph;
    }

    void Device::DestroyRenderGraph(RenderGraph* renderGraph)
    {
        ZoneScoped;
        TL_ASSERT(renderGraph != nullptr, "Cannot destroy a null render graph");
        renderGraph->Shutdown();
        delete renderGraph;
    }

} // namespace RHI