#pragma once
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#include "RHI/Backend/Vulkan/Context.hpp"
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
        virtual IContext*                         GetContext() override;
        virtual Expected<FencePtr>                CreateFence() override;
        virtual Expected<SwapChainPtr>            CreateSwapChain(const SwapChainDesc&) override;
        virtual Expected<RenderTargetPtr>         CreateRenderTarget(const RenderTargetDesc&) override;
        virtual Expected<SamplerPtr>              CreateSampler(const SamplerDesc&) override;
        virtual Expected<DescriptorsAllocatorPtr> CreateDescriptorAllocator(const DescriptorsAllocatorDesc&) override;
        virtual Expected<PipelineLayoutPtr>       CreatePipelineLayout(const PipelineLayoutDesc&) override;
        virtual Expected<PipelineStatePtr>        CreateGraphicsPipelineState(const GraphicsPipelineStateDesc&) override;
        virtual Expected<PipelineStatePtr>        CreateComputePipelineState(const ComputePipelineStateDesc&) override;
        virtual Expected<CommandsAllocatorPtr>    CreateCommandsAllocator() override;
        virtual Expected<BufferPtr>               CreateBuffer(const DeviceMemoryAllocationDesc&, const BufferDesc&) override;
        virtual Expected<BufferViewPtr>           CreateBufferView(const BufferViewDesc&) override;
        virtual Expected<TexturePtr>              CreateTexture(const DeviceMemoryAllocationDesc&, const TextureDesc&) override;
        virtual Expected<TextureViewPtr>          CreateTextureView(const TextureViewDesc&) override;

    private:
        VkResult CreateSurfaceIfNotExist(NativeWindowHandle _nativeWindowHandle, VkSurfaceKHR* _outSurface);
        VkResult CreateRenderPassIfNotExist(const RenderTargetDesc& _desc, VkRenderPass* _outRenderPass);

    private:
        VkInstance                                           m_instance;
        VkDebugUtilsMessengerEXT                             m_debugMessenger;
        IDebugMessenger*	                                 m_pDebugMessengerCallback;
        Unique<Device>                                       m_device;
        Unique<Context>                                      m_context;
        std::unordered_map<NativeWindowHandle, VkSurfaceKHR> m_surfaceMap;
        std::unordered_map<size_t, VkRenderPass>             m_renderPassMap;
    };

} // namespace Vulkan
} // namespace RHI
