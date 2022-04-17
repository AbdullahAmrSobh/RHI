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
        
        inline virtual Device& GetFactoryDevice() override { return *m_device; }

        // Interface Implementation.
        virtual Expected<SwapChainPtr> CreateSwapChain(const SwapChainDesc&) override;

        virtual Expected<ShaderModulePtr> CreateShaderModule(const ShaderModuleDesc&) override;

        virtual Expected<SamplerPtr> CreateSampler(const SamplerDesc&) override;

        virtual Expected<DescriptorSetLayoutPtr> CreateDescriptorSetLayout(const DescriptorSeLayoutDesc&) override;

        virtual Expected<DescriptorPoolPtr> CreateDescriptorPool(const DescriptorPoolDesc&) override;

        virtual Expected<PipelineLayoutPtr> CreatePipelineLayout(const PipelineLayoutDesc&) override;

        virtual Expected<PipelineStatePtr> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc&) override;

        virtual Expected<PipelineStatePtr> CreateComputePipelineState(const ComputePipelineStateDesc&) override;
	
		// virtual Expected<MemoryPoolPtr> CreateResourceMemoryPool(const MemoryPoolDesc&) override;
        
        virtual Expected<BufferPtr> CreateBuffer(const MemoryAllocationDesc&, const BufferDesc&) override;
        
        virtual Expected<BufferViewPtr> CreateBufferView(const BufferViewDesc&) override;
        
        virtual Expected<ImagePtr> CreateImage(const MemoryAllocationDesc&, const ImageDesc&) override;
        
        virtual Expected<ImageViewPtr> CreateImageView(const ImageViewDesc&) override;
        
        virtual Expected<FencePtr> CreateFence() override;
            
		virtual Expected<FrameGraphPtr> CreateFrameGraph() override;

    private:
        VkInstance               m_instance;
        VkDebugUtilsMessengerEXT m_debugMessenger;
        IDebugMessenger*         m_pDebugMessengerCallback;
        Unique<Device>           m_device;
    };

} // namespace Vulkan
} // namespace RHI
