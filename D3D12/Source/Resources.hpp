#pragma once

#include <RHI/Device.hpp>
#include <RHI/Resources.hpp>
#include <RHI/Result.hpp>
#include <RHI/Swapchain.hpp>

#include <TL/Containers/Vector.hpp>

#include <D3D12MemAlloc.h>
#include <d3d12.h>
#include <dxgi1_6.h>

namespace RHI::D3D12
{
    class IDevice;

    struct IFence : Fence
    {
        ID3D12Fence* fence = nullptr;
        HANDLE       event = nullptr;
        uint64_t     value = 0;

        ResultCode Init(IDevice* device, const FenceCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroupLayout : BindGroupLayout
    {
        TL::Vector<ShaderBinding> shaderBindings;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBindGroup : BindGroup
    {
        ID3D12DescriptorHeap* heap   = nullptr;
        IBindGroupLayout*     layout = nullptr;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    struct IShaderModule : ShaderModule
    {
        TL::Vector<uint8_t> code;

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        D3D12_SHADER_BYTECODE GetShaderBytecode() const;
    };

    struct IPipelineLayout : PipelineLayout
    {
        ID3D12RootSignature* rootSignature = nullptr;

        ResultCode Init(IDevice* device, const PipelineLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IGraphicsPipeline : GraphicsPipeline
    {
        ID3D12PipelineState* pipelineState = nullptr;
        IPipelineLayout*     layout        = nullptr;

        ResultCode Init(IDevice* device, const GraphicsPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IComputePipeline : ComputePipeline
    {
        ID3D12PipelineState* pipelineState = nullptr;
        IPipelineLayout*     layout        = nullptr;

        ResultCode Init(IDevice* device, const ComputePipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IRayTracingPipeline : RayTracingPipeline
    {
        ID3D12StateObject* stateObject = nullptr;
        IPipelineLayout*   layout      = nullptr;

        ResultCode Init(IDevice* device, const RayTracingPipelineCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void GetShaderBindingTableEntry(IDevice* device, uint32_t group, size_t size, void* dstHandle);
    };

    struct IQueryPool : QueryPool
    {
        ID3D12QueryHeap* heap = nullptr;
        QueryType        type = {};

        ResultCode Init(IDevice* device, const QueryPoolCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IBuffer : Buffer
    {
        D3D12MA::Allocation* allocation = nullptr;
        ID3D12Resource*      resource   = nullptr;

        ResultCode Init(IDevice* device, const BufferCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        DeviceMemoryPtr Map(IDevice* device);
        void            Unmap(IDevice* device);
    };

    struct IImage : Image
    {
        D3D12MA::Allocation* allocation = nullptr;
        ID3D12Resource*      resource   = nullptr;

        // TODO: the following should be removed
        Format                format           = Format::Unknown;
        ImageType             type             = {};
        ImageSubresourceRange subresourceRange = {};
        bool                  isView           = false;

        D3D12_CPU_DESCRIPTOR_HANDLE rtvDsvHandle    = {};
        D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHandle = {};

        ResultCode Init(IDevice* device, const ImageCreateInfo& createInfo);
        ResultCode Init(IDevice* device, const ImageViewCreateInfo& createInfo);
        ResultCode InitFromNative(IDevice* device, ID3D12Resource* resource, Format format, ImageSize2D size);
        void       Shutdown(IDevice* device);
    };

    struct ISampler : Sampler
    {
        D3D12_SAMPLER_DESC desc = {};

        ResultCode Init(IDevice* device, const SamplerCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IAccelerationStructure : AccelerationStructure
    {
        D3D12MA::Allocation*      allocation = nullptr;
        ID3D12Resource*           resource   = nullptr;
        D3D12_GPU_VIRTUAL_ADDRESS address    = 0;

        AccelerationStructureSizesInfo sizes = {};

        ResultCode Init(IDevice* device, const AccelerationStructureCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    struct IMicromap : Micromap
    {
        D3D12MA::Allocation* allocation = nullptr;
        ID3D12Resource*      resource   = nullptr;

        ResultCode Init(IDevice* device, const MicromapCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    class ISwapchain final : public Swapchain
    {
    public:
        constexpr static auto MaxImageCount = 4;

        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        // Interface
        uint32_t               GetImagesCount() const override;
        SwapchainAcquireResult AcquireImage() override;
        SurfaceCapabilities    GetSurfaceCapabilities() const override;
        ResultCode             Resize(const ImageSize2D& size) override;
        ResultCode             Configure(const SwapchainConfigureInfo& configInfo) override;

        ResultCode Present();

        IDevice*               m_device                = nullptr;
        IDXGISwapChain4*       m_swapchain             = nullptr;
        uint32_t               m_imageIndex            = 0;
        uint32_t               m_imageCount            = 0;
        IImage*                m_images[MaxImageCount] = {};
        IFence                 m_acquireFences[MaxImageCount] = {};
        SwapchainConfigureInfo m_configuration         = {};
    };
} // namespace RHI::D3D12
