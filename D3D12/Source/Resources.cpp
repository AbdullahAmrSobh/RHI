#include "Resources.hpp"

#include "Device.hpp"

namespace RHI::D3D12
{
    ///////////////////////////////////////////////////////////
    // IFence
    ///////////////////////////////////////////////////////////

    ResultCode IFence::Init(IDevice* device, const FenceCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IFence::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IBindGroupLayout
    ///////////////////////////////////////////////////////////

    ResultCode IBindGroupLayout::Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IBindGroupLayout::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IBindGroup
    ///////////////////////////////////////////////////////////

    ResultCode IBindGroup::Init(IDevice* device, const BindGroupCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IBindGroup::Shutdown(IDevice* device)
    {
        (void)device;
    }

    void IBindGroup::Update(IDevice* device, const BindGroupUpdateInfo& updateInfo)
    {
        (void)device;
        (void)updateInfo;
    }

    ///////////////////////////////////////////////////////////
    // IShaderModule
    ///////////////////////////////////////////////////////////

    ResultCode IShaderModule::Init(IDevice* device, const ShaderModuleCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IShaderModule::Shutdown(IDevice* device)
    {
        (void)device;
    }

    D3D12_SHADER_BYTECODE IShaderModule::GetShaderBytecode() const
    {
        return {};
    }

    ///////////////////////////////////////////////////////////
    // IPipelineLayout
    ///////////////////////////////////////////////////////////

    ResultCode IPipelineLayout::Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IPipelineLayout::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IGraphicsPipeline
    ///////////////////////////////////////////////////////////

    ResultCode IGraphicsPipeline::Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IGraphicsPipeline::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IComputePipeline
    ///////////////////////////////////////////////////////////

    ResultCode IComputePipeline::Init(IDevice* device, const ComputePipelineCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IComputePipeline::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IRayTracingPipeline
    ///////////////////////////////////////////////////////////

    ResultCode IRayTracingPipeline::Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IRayTracingPipeline::Shutdown(IDevice* device)
    {
        (void)device;
    }

    void IRayTracingPipeline::GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle)
    {
        (void)device;
        (void)group;
        (void)size;
        (void)dstHandle;
    }

    ///////////////////////////////////////////////////////////
    // IQueryPool
    ///////////////////////////////////////////////////////////

    ResultCode IQueryPool::Init(IDevice* device, const QueryPoolCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IQueryPool::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IBuffer
    ///////////////////////////////////////////////////////////

    ResultCode IBuffer::Init(IDevice* device, const BufferCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IBuffer::Shutdown(IDevice* device)
    {
        (void)device;
    }

    DeviceMemoryPtr IBuffer::Map(IDevice* device)
    {
        (void)device;
        return nullptr;
    }

    void IBuffer::Unmap(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IImage
    ///////////////////////////////////////////////////////////

    ResultCode IImage::Init(IDevice* device, const ImageCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    ResultCode IImage::Init(IDevice* device, const ImageViewCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    ResultCode IImage::InitFromNative(IDevice* device, ID3D12Resource* resource, Format format, ImageSize2D size)
    {
        (void)device;
        (void)resource;
        (void)format;
        (void)size;
        return ResultCode::Success;
    }

    void IImage::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // ISampler
    ///////////////////////////////////////////////////////////

    ResultCode ISampler::Init(IDevice* device, const SamplerCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void ISampler::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IAccelerationStructure
    ///////////////////////////////////////////////////////////

    ResultCode IAccelerationStructure::Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IAccelerationStructure::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // IMicromap
    ///////////////////////////////////////////////////////////

    ResultCode IMicromap::Init(IDevice* device, const MicromapCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void IMicromap::Shutdown(IDevice* device)
    {
        (void)device;
    }

    ///////////////////////////////////////////////////////////
    // ISwapchain
    ///////////////////////////////////////////////////////////

    ISwapchain::ISwapchain()  = default;
    ISwapchain::~ISwapchain() = default;

    ResultCode ISwapchain::Init(IDevice* device, const SwapchainCreateInfo& createInfo)
    {
        (void)device;
        (void)createInfo;
        return ResultCode::Success;
    }

    void ISwapchain::Shutdown(IDevice* device)
    {
        (void)device;
    }

    uint32_t ISwapchain::GetImagesCount() const
    {
        return 0;
    }

    SwapchainAcquireResult ISwapchain::AcquireImage()
    {
        return {};
    }

    SurfaceCapabilities ISwapchain::GetSurfaceCapabilities() const
    {
        return {};
    }

    ResultCode ISwapchain::Resize(const ImageSize2D& size)
    {
        (void)size;
        return ResultCode::Success;
    }

    ResultCode ISwapchain::Configure(const SwapchainConfigureInfo& configInfo)
    {
        (void)configInfo;
        return ResultCode::Success;
    }

    ResultCode ISwapchain::Present()
    {
        return ResultCode::Success;
    }
} // namespace RHI::D3D12
