#pragma once
#include "RHI/Commands.hpp"
#include "RHI/Context.hpp"
#include "RHI/Definitions.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

using FactoryPtr = Unique<class IFactory>;

class IFactory
{
public:
    static Expected<FactoryPtr> Create(EBackendType, IDebugMessenger&);
    
    virtual ~IFactory() = default;

    virtual IContext*                         GetContext()                                                         = 0;
    virtual Expected<FencePtr>                CreateFence()                                                        = 0;
    virtual Expected<SwapChainPtr>            CreateSwapChain(const SwapChainDesc&)                                = 0;
    virtual Expected<RenderTargetPtr>         CreateRenderTarget(const RenderTargetDesc&)                          = 0;
    virtual Expected<SamplerPtr>              CreateSampler(const SamplerDesc&)                                    = 0;
    virtual Expected<DescriptorsAllocatorPtr> CreateDescriptorAllocator(const DescriptorsAllocatorDesc&)           = 0;
    virtual Expected<PipelineLayoutPtr>       CreatePipelineLayout(const PipelineLayoutDesc&)                      = 0;
    virtual Expected<PipelineStatePtr>        CreateGraphicsPipelineState(const GraphicsPipelineStateDesc&)        = 0;
    virtual Expected<PipelineStatePtr>        CreateComputePipelineState(const ComputePipelineStateDesc&)          = 0;
    virtual Expected<CommandsAllocatorPtr>    CreateCommandsAllocator()                                            = 0;
    virtual Expected<BufferPtr>               CreateBuffer(const DeviceMemoryAllocationDesc&, const BufferDesc&)   = 0;
    virtual Expected<BufferViewPtr>           CreateBufferView(const BufferViewDesc&)                              = 0;
    virtual Expected<TexturePtr>              CreateTexture(const DeviceMemoryAllocationDesc&, const TextureDesc&) = 0;
    virtual Expected<TextureViewPtr>          CreateTextureView(const TextureViewDesc&)                            = 0;
};

} // namespace RHI
