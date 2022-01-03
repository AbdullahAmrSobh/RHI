#pragma once
#define NOMINMAX
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Factory.hpp"

namespace RHI
{
namespace Vulkan
{
    class Factory final : public IFactory
    {
    public:
        Factory() = default;
        ~Factory();

        EResultCode Init(VkApplicationInfo _appInfo, IDebugMessenger& debugMessengerCallback);

        // Interface Implementation.
        virtual Expected<FencePtr>             CreateFence() override;
        virtual Expected<SwapChainPtr>         CreateSwapChain(const SwapChainDesc&) override;
        virtual Expected<RenderTargetPtr>      CreateRenderTarget(const RenderTargetDesc&) override;
        virtual Expected<SamplerPtr>           CreateSampler(const SamplerDesc&) override;
        virtual Expected<IDescriptorPool>      CreateDescriptorPool(const DescriptorPoolDesc&) override;
        virtual Expected<PipelineLayoutPtr>    CreatePipelineLayout(const PipelineLayoutDesc&) override;
        virtual Expected<PipelineStatePtr>     CreateGraphicsPipelineState(const GraphicsPipelineStateDesc&) override;
        virtual Expected<PipelineStatePtr>     CreateComputePipelineState(const ComputePipelineStateDesc&) override;
        virtual Expected<CommandsAllocatorPtr> CreateCommandsAllocator() override;
        virtual Expected<BufferPtr>            CreateBuffer(const MemoryAllocationDesc&, const BufferDesc&) override;
        virtual Expected<BufferViewPtr>        CreateBufferView(const BufferViewDesc&) override;
        virtual Expected<TexturePtr>           CreateTexture(const MemoryAllocationDesc&, const TextureDesc&) override;
        virtual Expected<TextureViewPtr>       CreateTextureView(const TextureViewDesc&) override;

    private:
        // VkResult CreateSurfaceIfNotExist(NativeWindowHandle _nativeWindowHandle, VkSurfaceKHR* _outSurface);
        // VkResult CreateRenderPassIfNotExist(const RenderTargetDesc& _desc, VkRenderPass* _outRenderPass);

    private:
        VkInstance               m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        IDebugMessenger*         m_pDebugMessengerCallback;
        Unique<Device>           m_device;
        // std::unordered_map<NativeWindowHandle, VkSurfaceKHR> m_surfaceMap;
        // std::unordered_map<size_t, VkRenderPass>             m_renderPassMap;
    };

} // namespace Vulkan
} // namespace RHI
