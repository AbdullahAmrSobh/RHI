#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/PipelineState.hpp"

namespace RHI
{
namespace Vulkan
{
    class PipelineState final
        : public IPipelineState
        , public DeviceObject<VkPipeline>
    {
    public:
        PipelineState(Device& device)
            : DeviceObject(device)
        {
        }
        ~PipelineState();
        
        VkResult Init(const GraphicsPipelineStateDesc& desc);
        VkResult Init(const ComputePipelineStateDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI
