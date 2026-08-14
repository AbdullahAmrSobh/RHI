#include "resources.h"

namespace RHI::WebGPU
{
    ResultCode IFence::Init(IDevice* device, const FenceCreateInfo& createInfo)
    {
        return {};
    }

    void IFence::Shutdown(IDevice* device)
    {
    }

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        return {};
    }

    void IBindGroupLayout::Shutdown(IDevice* device)
    {
    }

    ResultCode IBindGroup::Init(IDevice* device, const BindGroupCreateInfo& createInfo)
    {
        return {};
    }

    void IBindGroup::Shutdown(IDevice* device)
    {
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
    }

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        return {};
    }

    void IShaderModule::Shutdown(IDevice* device)
    {
    }

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        return {};
    }

    void IPipelineLayout::Shutdown(IDevice* device)
    {
    }

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        return {};
    }

    void IGraphicsPipeline::Shutdown(IDevice* device)
    {
    }

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        return {};
    }

    void IComputePipeline::Shutdown(IDevice* device)
    {
    }

    ResultCode IRayTracingPipeline::Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo)
    {
        return {};
    }

    void IRayTracingPipeline::Shutdown(IDevice* device)
    {
    }

    void IRayTracingPipeline::GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle)
    {
    }

    ResultCode IQueryPool::Init(IDevice* device, const QueryPoolCreateInfo& createInfo)
    {
        return {};
    }

    void IQueryPool::Shutdown(IDevice* device)
    {
    }

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        return {};
    }

    void IBuffer::Shutdown(IDevice* device)
    {
    }

    DeviceMemoryPtr IBuffer::Map(IDevice* device)
    {
        return {};
    }

    void IBuffer::Unmap(IDevice* device)
    {
    }

    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        return {};
    }

    ResultCode IImage::Init(IDevice* device, const ImageViewCreateInfo& createInfo)
    {
        return {};
    }

    void IImage::Shutdown(IDevice* device)
    {
    }

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        return {};
    }

    void ISampler::Shutdown(IDevice* device)
    {
    }

    ResultCode IAccelerationStructure::Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo)
    {
        return {};
    }

    void IAccelerationStructure::Shutdown(IDevice* device)
    {
    }

    ResultCode IMicromap::Init(IDevice* device, const MicromapCreateInfo& createInfo)
    {
        return {};
    }

    void IMicromap::Shutdown(IDevice* device)
    {
    }

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        return {};
    }

    void ISwapchain::Shutdown(IDevice* device)
    {
    }

    uint32_t ISwapchain::GetImagesCount() const
    {
        return {};
    }

    SwapchainAcquireResult ISwapchain::AcquireSwapchainImage()
    {
        return {};
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities(IDevice* device) const
    {
        return {};
    }

    ResultCode ISwapchain::ResizeSwapchain(IDevice* device, const ImageSize2D& size)
    {
        return {};
    }

    ResultCode ISwapchain::ConfigureSwapchain(IDevice* device, const SwapchainConfigureInfo& configInfo)
    {
        return {};
    }

} // namespace RHI::WebGPU
