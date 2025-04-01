#pragma once

#include <RHI/Device.hpp>
#include <RHI/Queue.hpp>

#include <TL/Containers.hpp>

#include <RHI-WebGPU/Loader.hpp>
#include <webgpu/webgpu.h>

// #include "CommandList.hpp"
#include "Resources.hpp"

namespace RHI::WebGPU
{
    class IDevice final : public Device
    {
    private:
        friend Device* RHI::CreateWebGPUDevice();
        friend void    RHI::DestroyWebGPUDevice(Device* device);

    public:
        IDevice();
        ~IDevice();

        ResultCode Init();
        void       Shutdown();

        // Interface Implementation
        RenderGraph*             CreateRenderGraph(const RenderGraphCreateInfo& createInfo) override;
        void                     DestroyRenderGraph(RenderGraph* renderGraph) override;
        Swapchain*               CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void                     DestroySwapchain(Swapchain* swapchain) override;
        ShaderModule*            CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void                     DestroyShaderModule(ShaderModule* shaderModule) override;
        CommandList*             CreateCommandList(const CommandListCreateInfo& createInfo) override;
        void                     DestroyCommandList(CommandList*);
        Handle<BindGroupLayout>  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void                     DestroyBindGroupLayout(Handle<BindGroupLayout> handle) override;
        Handle<BindGroup>        CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void                     DestroyBindGroup(Handle<BindGroup> handle) override;
        void                     UpdateBindGroup(Handle<BindGroup> handle, const BindGroupUpdateInfo& updateInfo) override;
        Handle<PipelineLayout>   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void                     DestroyPipelineLayout(Handle<PipelineLayout> handle) override;
        Handle<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void                     DestroyGraphicsPipeline(Handle<GraphicsPipeline> handle) override;
        Handle<ComputePipeline>  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void                     DestroyComputePipeline(Handle<ComputePipeline> handle) override;
        Handle<Sampler>          CreateSampler(const SamplerCreateInfo& createInfo) override;
        void                     DestroySampler(Handle<Sampler> handle) override;
        Result<Handle<Image>>    CreateImage(const ImageCreateInfo& createInfo) override;
        Handle<Image>            CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void                     DestroyImage(Handle<Image> handle) override;
        Result<Handle<Buffer>>   CreateBuffer(const BufferCreateInfo& createInfo) override;
        void                     DestroyBuffer(Handle<Buffer> handle) override;
        void                     BeginResourceUpdate(RenderGraph* renderGraph) override;
        void                     EndResourceUpdate() override;
        void                     BufferWrite(Handle<Buffer> buffer, size_t offset, TL::Block block) override;
        void                     ImageWrite(Handle<Image> image, ImageOffset3D offset, ImageSize3D size, uint32_t mipLevel, uint32_t arrayLayer, TL::Block block) override;
        void                     CollectResources() override;

    public:
        void ExecuteCommandList(class ICommandList* commandList);

    public:
        /// @todo: everything here should be made private
        TL::Arena                     m_tempAllocator = TL::Arena();
        // Vulkan instance and core objects
        WGPUInstance                  m_instance; ///< WGPU instance handle.
        WGPUAdapter                   m_adapter;  ///< Physical device selected for use.
        WGPUDevice                    m_device;   ///< Logical device handle.
        // Queue and allocator management
        WGPUQueue                     m_queue;
        // Resource pools
        HandlePool<IImage>            m_imageOwner;            ///< Pool for managing image resource handles.
        HandlePool<IBuffer>           m_bufferOwner;           ///< Pool for managing buffer resource handles.
        HandlePool<IBindGroupLayout>  m_bindGroupLayoutsOwner; ///< Pool for managing bind group layout handles.
        HandlePool<IBindGroup>        m_bindGroupOwner;        ///< Pool for managing bind group handles.
        HandlePool<IPipelineLayout>   m_pipelineLayoutOwner;   ///< Pool for managing pipeline layout handles.
        HandlePool<IGraphicsPipeline> m_graphicsPipelineOwner; ///< Pool for managing graphics pipeline handles.
        HandlePool<IComputePipeline>  m_computePipelineOwner;  ///< Pool for managing compute pipeline handles.
        HandlePool<ISampler>          m_samplerOwner;          ///< Pool for managing sampler handles.
        // // Frame
        std::atomic_uint64_t          m_frameIndex;

        TL::Set<WGPUBuffer> m_buffersToUnmap;
    };
} // namespace RHI::WebGPU