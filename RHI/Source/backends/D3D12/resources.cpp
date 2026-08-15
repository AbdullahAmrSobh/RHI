#include <backends/D3D12/resources.h>

namespace RHI::D3D12
{
    void IFence::Init(IDevice*, const FenceCreateInfo&) {}

    void IFence::Shutdown(IDevice*) {}

    void IBindGroupLayout::Init(IDevice*, const BindGroupLayoutCreateInfo&) {}

    void IBindGroupLayout::Shutdown(IDevice*) {}

    void IBindGroup::Init(IDevice*, const BindGroupCreateInfo&) {}

    void IBindGroup::Shutdown(IDevice*) {}

    void IBindGroup::Update(IDevice*, const BindGroupUpdateInfo&) {}

    void IShaderModule::Init(IDevice*, const ShaderModuleCreateInfo&) {}

    void IShaderModule::Shutdown(IDevice*) {}

    void IPipelineLayout::Init(IDevice*, const PipelineLayoutCreateInfo&) {}

    void IPipelineLayout::Shutdown(IDevice*) {}

    void IGraphicsPipeline::Init(IDevice*, const GraphicsPipelineCreateInfo&) {}

    void IGraphicsPipeline::Shutdown(IDevice*) {}

    void IComputePipeline::Init(IDevice*, const ComputePipelineCreateInfo&) {}

    void IComputePipeline::Shutdown(IDevice*) {}

    void IRayTracingPipeline::Init(IDevice*, const RayTracingPipelineCreateInfo&) {}

    void IRayTracingPipeline::Shutdown(IDevice*) {}

    void IRayTracingPipeline::GetShaderBindingTableEntry(IDevice*, uint32_t, size_t, void*) {}

    void IQueryPool::Init(IDevice*, const QueryPoolCreateInfo&) {}

    void IQueryPool::Shutdown(IDevice*) {}

    void IBuffer::Init(IDevice*, const BufferCreateInfo&) {}

    void IBuffer::Shutdown(IDevice*) {}

    DeviceMemoryPtr IBuffer::Map(IDevice*)
    {
        return nullptr;
    }

    void IBuffer::Unmap(IDevice*) {}

    void IImage::Init(IDevice*, const ImageCreateInfo&) {}

    void IImage::Init(IDevice*, const ImageViewCreateInfo&) {}

    void IImage::Shutdown(IDevice*) {}

    void ISampler::Init(IDevice*, const SamplerCreateInfo&) {}

    void ISampler::Shutdown(IDevice*) {}

    void IAccelerationStructure::Init(IDevice*, const AccelerationStructureCreateInfo&) {}

    void IAccelerationStructure::Shutdown(IDevice*) {}

    void IMicromap::Init(IDevice*, const MicromapCreateInfo&) {}

    void IMicromap::Shutdown(IDevice*) {}

    void ISwapchain::Init(IDevice*, const SwapchainCreateInfo&) {}

    void ISwapchain::Shutdown(IDevice*) {}
} // namespace RHI::D3D12
