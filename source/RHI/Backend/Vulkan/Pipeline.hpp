#pragma once

#include "RHI/Backend/Vulkan/Vulkan.hpp"
#include "RHI/Pipeline.hpp"

namespace Vulkan
{
class Context;

class RHI_EXPORT PipelineState final
    : public RHI::PipelineState
    , public DeviceObject<vk::Pipeline>
{
public:
    PipelineState(RHI::Context& context)
        : RHI::PipelineState(context)
    {
    }

    ~PipelineState();

    RHI::ResultCode Init(const RHI::GraphicsPipelineCreateInfo& createInfo) override;
    RHI::ResultCode Init(const RHI::ComputePipelineCreateInfo& createInfo) override;
    RHI::ResultCode Init(const RHI::RayTracingPipelineCreateInfo& createInfo) override;

    std::shared_ptr<vk::UniquePipelineLayout> m_layout;
};

}  // namespace Vulkan