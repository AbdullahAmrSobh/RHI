#pragma once

#include <RHI/Device.hpp>
#include <RHI/Queue.hpp>

#include <TL/Containers/Containers.hpp>

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
        uint64_t          GetNativeHandle(NativeHandleType type, uint64_t handle) override;
        Swapchain*        CreateSwapchain(const SwapchainCreateInfo& createInfo) override;
        void              DestroySwapchain(Swapchain* swapchain) override;
        ShaderModule*     CreateShaderModule(const ShaderModuleCreateInfo& createInfo) override;
        void              DestroyShaderModule(ShaderModule* shaderModule) override;
        BindGroupLayout*  CreateBindGroupLayout(const BindGroupLayoutCreateInfo& createInfo) override;
        void              DestroyBindGroupLayout(BindGroupLayout* handle) override;
        BindGroup*        CreateBindGroup(const BindGroupCreateInfo& createInfo) override;
        void              DestroyBindGroup(BindGroup* handle) override;
        void              UpdateBindGroup(BindGroup* handle, const BindGroupUpdateInfo& updateInfo) override;
        Buffer*           CreateBuffer(const BufferCreateInfo& createInfo) override;
        void              DestroyBuffer(Buffer* handle) override;
        Image*            CreateImage(const ImageCreateInfo& createInfo) override;
        Image*            CreateImageView(const ImageViewCreateInfo& createInfo) override;
        void              DestroyImage(Image* handle) override;
        Sampler*          CreateSampler(const SamplerCreateInfo& createInfo) override;
        void              DestroySampler(Sampler* handle) override;
        PipelineLayout*   CreatePipelineLayout(const PipelineLayoutCreateInfo& createInfo) override;
        void              DestroyPipelineLayout(PipelineLayout* handle) override;
        GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
        void              DestroyGraphicsPipeline(GraphicsPipeline* handle) override;
        ComputePipeline*  CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
        void              DestroyComputePipeline(ComputePipeline* handle) override;
        ResultCode        SetFramesInFlightCount(uint32_t count) override;
        Frame*            GetCurrentFrame() override;

    public:
        /// @todo: everything here should be made private
        TL::Arena    m_tempAllocator = TL::Arena();
        // Vulkan instance and core objects
        WGPUInstance m_instance; ///< WGPU instance handle.
        WGPUAdapter  m_adapter;  ///< Physical device selected for use.
        WGPUDevice   m_device;   ///< Logical device handle.

        // Queue and allocator management
        WGPUQueue    m_queue;

    private:
        TL::Map<Swapchain*, TL::Stacktrace>        m_liveSwapchains;
        TL::Map<ShaderModule*, TL::Stacktrace>     m_liveShaderModules;
        TL::Map<Image*, TL::Stacktrace>            m_liveImages;
        TL::Map<Buffer*, TL::Stacktrace>           m_liveBuffers;
        TL::Map<BindGroupLayout*, TL::Stacktrace>  m_liveBindGroupLayouts;
        TL::Map<BindGroup*, TL::Stacktrace>        m_liveBindGroups;
        TL::Map<PipelineLayout*, TL::Stacktrace>   m_livePipelineLayouts;
        TL::Map<GraphicsPipeline*, TL::Stacktrace> m_liveGraphicsPipelines;
        TL::Map<ComputePipeline*, TL::Stacktrace>  m_liveComputePipelines;
        TL::Map<Sampler*, TL::Stacktrace>          m_liveSamplers;

        // Frame
        std::atomic_uint64_t                       m_frameIndex;
    };
} // namespace RHI::WebGPU