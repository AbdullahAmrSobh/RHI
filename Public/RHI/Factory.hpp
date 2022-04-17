#pragma once
#include "Device.hpp"
#include "RHI/Buffer.hpp"
#include "RHI/Commands.hpp"
#include "RHI/Debug.hpp"
#include "RHI/DescriptorPool.hpp"
#include "RHI/Fence.hpp"
#include "RHI/FrameGraph.hpp"
#include "RHI/Image.hpp"
#include "RHI/PipelineLayout.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/RenderTargetLayout.hpp"
#include "RHI/Sampler.hpp"
#include "RHI/SwapChain.hpp"

namespace RHI
{
using FactoryPtr = Unique<class IFactory>;
class IFactory
{
public:
    static Expected<FactoryPtr> Create(EBackendType, IDebugMessenger&);

    virtual ~IFactory() = default;

    virtual IDevice& GetFactoryDevice() = 0;

    virtual Expected<SwapChainPtr> CreateSwapChain(const SwapChainDesc&) = 0;

    virtual Expected<ShaderModulePtr> CreateShaderModule(const ShaderModuleDesc&) = 0;

    virtual Expected<SamplerPtr> CreateSampler(const SamplerDesc&) = 0;

    virtual Expected<DescriptorSetLayoutPtr> CreateDescriptorSetLayout(const DescriptorSeLayoutDesc&) = 0;

    virtual Expected<DescriptorPoolPtr> CreateDescriptorPool(const DescriptorPoolDesc&) = 0;

    virtual Expected<PipelineLayoutPtr> CreatePipelineLayout(const PipelineLayoutDesc&) = 0;

    virtual Expected<PipelineStatePtr> CreateGraphicsPipelineState(const GraphicsPipelineStateDesc&) = 0;

    virtual Expected<PipelineStatePtr> CreateComputePipelineState(const ComputePipelineStateDesc&) = 0;
    
    // virtual Expected<PipelineStatePtr> CreateRayTracingPipeline(const RayTracingPipelineDesc&) = 0;

    // virtual Expected<MemoryPoolPtr> CreateResourceMemoryPool(const MemoryPoolDesc&) = 0;
    
    virtual Expected<BufferPtr> CreateBuffer(const MemoryAllocationDesc&, const BufferDesc&) = 0;

    virtual Expected<BufferViewPtr> CreateBufferView(const BufferViewDesc&) = 0;

    virtual Expected<ImagePtr> CreateImage(const MemoryAllocationDesc&, const ImageDesc&) = 0;
    
    virtual Expected<ImageViewPtr> CreateImageView(const ImageViewDesc&) = 0;
    
    // virtual Expected<AccelerationStructurePtr> CreateAccelerationStructure(const AccelerationStructureDesc&) = 0;
    
    virtual Expected<FencePtr> CreateFence() = 0;
    
    virtual Expected<FrameGraphPtr> CreateFrameGraph() = 0;
};

} // namespace RHI
