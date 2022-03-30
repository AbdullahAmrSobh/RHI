#pragma once
#include "Device.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/Commands.hpp"
#include "RHI/Debug.hpp"
#include "RHI/Definitions.hpp"
#include "RHI/DescriptorPool.hpp"
#include "RHI/Fence.hpp"
#include "RHI/PipelineLayout.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/RenderGraph.hpp"
#include "RHI/RenderGraphBuilder.hpp"
#include "RHI/RenderTarget.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Sampler.hpp"
#include "RHI/SwapChain.hpp"
#include "RHI/Texture.hpp"

#define HANDLE_OBJECT_CREATION_FAIL [](RHI::EResultCode result) { HZ_CORE_ASSERT(result == RHI::EResultCode::Success, "Failed to create object"); }

namespace RHI
{
using FactoryPtr = Unique<class IFactory>;

class IFactory
{
public:
    static Expected<FactoryPtr> Create(EBackendType, IDebugMessenger&);

    virtual ~IFactory() = default;

    virtual Device& GetFactoryDevice() = 0;
    
    virtual Expected<SwapChainPtr> CreateSwapChain(const SwapChainDesc&) = 0;

    virtual Expected<ShaderModulePtr> CreateShaderModule(const ShaderModuleDesc&) = 0;
    
    virtual Expected<SamplerPtr> CreateSampler(const SamplerDesc&) = 0;

    virtual Expected<DescriptorSetLayoutPtr> CreateDescriptorSetLayout(const DescriptorSeLayoutDesc&) = 0;

    virtual Expected<DescriptorPoolPtr> CreateDescriptorPool(const DescriptorPoolDesc&) = 0;

    virtual Expected<PipelineLayoutPtr> CreatePipelineLayout(const PipelineLayoutDesc&) = 0;

    virtual Expected<PipelineStatePtr> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc&) = 0;

    virtual Expected<PipelineStatePtr> CreateComputePipelineState(const ComputePipelineStateDesc&) = 0;

    virtual Expected<BufferPtr> CreateBuffer(const MemoryAllocationDesc&, const BufferDesc&) = 0;
    
    virtual Expected<BufferViewPtr> CreateBufferView(const BufferViewDesc&) = 0;
    
    virtual Expected<TexturePtr> CreateTexture(const MemoryAllocationDesc&, const TextureDesc&) = 0;

    virtual Expected<TextureViewPtr> CreateTextureView(const TextureViewDesc&) = 0;

    virtual Expected<FencePtr> CreateFence() = 0;
    
    virtual Expected<RenderGraphPtr> CreateRenderGraph(const RenderGraphBuilder&) = 0;
    
	virtual Expected<RenderTargetPtr> CreateRenderTarget(const RenderTargetDesc&) = 0;
    
    // Debug methods
    virtual void OnInit(void* pUserData)    {};
    virtual void OnUpdate(void* pUserData)  {};
    virtual void OnDestroy(void* pUserData) {};
};

} // namespace RHI
