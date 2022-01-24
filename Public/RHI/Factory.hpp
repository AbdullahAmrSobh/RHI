#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/Commands.hpp"
#include "RHI/Descriptor.hpp"
#include "RHI/Fence.hpp"
#include "RHI/PipelineLayout.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/RenderTarget.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Sampler.hpp"
#include "RHI/SwapChain.hpp"
#include "RHI/Texture.hpp"
#include "RHI/Debug.hpp"

#define HANDLE_OBJECT_CREATION_FAIL [](RHI::EResultCode result) { HZ_CORE_ASSERT(result == RHI::EResultCode::Success, "Failed to create object"); }

namespace RHI
{
using FactoryPtr = Unique<class IFactory>;

class IFactory
{
public:
    static Expected<FactoryPtr> Create(EBackendType, IDebugMessenger&);

    virtual ~IFactory() = default;

    virtual Expected<ShaderBytecodePtr>    CreateShaderBytecode(const ShaderBytecodeDesc&)                = 0;
    virtual Expected<FencePtr>             CreateFence()                                                  = 0;
    virtual Expected<SwapChainPtr>         CreateSwapChain(const SwapChainDesc&)                          = 0;
    virtual Expected<RenderTargetPtr>      CreateRenderTarget(const RenderTargetDesc&)                    = 0;
    virtual Expected<SamplerPtr>           CreateSampler(const SamplerDesc&)                              = 0;
    virtual Expected<DescriptorPoolPtr>    CreateDescriptorPool(const DescriptorPoolDesc&)                = 0;
    virtual Expected<PipelineLayoutPtr>    CreatePipelineLayout(const PipelineLayoutDesc&)                = 0;
    virtual Expected<PipelineStatePtr>     CreateGraphicsPipelineState(const GraphicsPipelineStateDesc&)  = 0;
    virtual Expected<PipelineStatePtr>     CreateComputePipelineState(const ComputePipelineStateDesc&)    = 0; 
    virtual Expected<CommandQueuePtr>      CreateCommandQueue(CommandQueueTypeFlags)                      = 0;
    virtual Expected<CommandsAllocatorPtr> CreateCommandsAllocator()                                      = 0;
    virtual Expected<BufferPtr>            CreateBuffer(const MemoryAllocationDesc&, const BufferDesc&)   = 0;
    virtual Expected<BufferViewPtr>        CreateBufferView(const BufferViewDesc&)                        = 0;
    virtual Expected<TexturePtr>           CreateTexture(const MemoryAllocationDesc&, const TextureDesc&) = 0;
    virtual Expected<TextureViewPtr>       CreateTextureView(const TextureViewDesc&)                      = 0;
};

} // namespace RHI
