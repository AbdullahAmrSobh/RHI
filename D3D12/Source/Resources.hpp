#pragma once

#include <RHI/CommandList.hpp>
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
    class ICommandList;

    ///////////////////////////////////////////////////////////
    // IFence
    ///////////////////////////////////////////////////////////

    struct IFence : Fence
    {
        ID3D12Fence* m_fence      = nullptr;
        HANDLE       m_event      = nullptr;

        ResultCode Init(IDevice* device, const FenceCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
    };

    ///////////////////////////////////////////////////////////
    // ICommandPool
    ///////////////////////////////////////////////////////////

    class ICommandPool final : public CommandPool
    {
    public:
        ResultCode   Init(IDevice* device, const CommandPoolCreateInfo& createInfo);
        void         Shutdown(IDevice* device);

        void         Reset() override;
        CommandList* Allocate() override;

        IDevice*                     m_device         = nullptr;
        ID3D12CommandAllocator*      m_allocator       = nullptr;
        D3D12_COMMAND_LIST_TYPE      m_type            = D3D12_COMMAND_LIST_TYPE_DIRECT;
        TL::Vector<ICommandList*>    m_commandLists;
    };

    ///////////////////////////////////////////////////////////
    // Descriptors
    ///////////////////////////////////////////////////////////

    struct IBindGroupLayout : BindGroupLayout
    {
        TL::Vector<ShaderBinding> imageBindings;
        TL::Vector<ShaderBinding> bufferBindings;
        TL::Vector<ShaderBinding> samplerBindings;

        ResultCode Init(IDevice* device, const BindGroupLayoutCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        ShaderBinding GetBindingInfo(int index) const;

        D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU(uint32_t binding, uint32_t arrayIndex) const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU(uint32_t binding, uint32_t arrayIndex) const;
    };

    struct IBindGroup : BindGroup
    {
        ID3D12DescriptorHeap* heap   = nullptr;
        IBindGroupLayout*     layout = nullptr;

        ResultCode Init(IDevice* device, const BindGroupCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        void Update(IDevice* device, const BindGroupUpdateInfo& updateInfo);
    };

    ///////////////////////////////////////////////////////////
    // Shaders and pipelines
    ///////////////////////////////////////////////////////////

    struct IShaderModule : ShaderModule
    {
        TL::Vector<uint8_t> code;

        D3D12_SHADER_BYTECODE GetShaderBytecode() const;

        ResultCode Init(IDevice* device, const ShaderModuleCreateInfo& createInfo);
        void       Shutdown(IDevice* device);
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
    };

    ///////////////////////////////////////////////////////////
    // Resources
    ///////////////////////////////////////////////////////////

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
    };

    struct IImage : Image
    {
        D3D12MA::Allocation* allocation = nullptr;
        ID3D12Resource*      resource   = nullptr;

        Format                format           = Format::Unknown;
        ImageType             type             = {};
        ImageSubresourceRange subresourceRange = {};
        bool                  isView           = false;

        D3D12_CPU_DESCRIPTOR_HANDLE rtvDsvHandle     = {};
        D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvUavHandle  = {};

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

    ///////////////////////////////////////////////////////////
    // ISwapchain
    ///////////////////////////////////////////////////////////

    class ISwapchain final : public Swapchain
    {
    public:
        constexpr static auto MaxImageCount = 4;

        ISwapchain();
        ~ISwapchain();

        ResultCode Init(IDevice* device, const SwapchainCreateInfo& createInfo);
        void       Shutdown(IDevice* device);

        // Interface
        uint32_t            GetImagesCount() const override;
        Image*              GetImage() const override;
        SurfaceCapabilities GetSurfaceCapabilities() const override;
        ResultCode          Resize(const ImageSize2D& size) override;
        ResultCode          Configure(const SwapchainConfigureInfo& configInfo) override;

        IDevice*              m_device     = nullptr;
        IDXGISwapChain4*      m_swapchain  = nullptr;
        uint32_t              m_imageIndex = 0;
        uint32_t              m_imageCount = 0;
        IImage*               m_images[MaxImageCount]  = {};
        SwapchainConfigureInfo m_configuration         = {};
    };

} // namespace RHI::D3D12
