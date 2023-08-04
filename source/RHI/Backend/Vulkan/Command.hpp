#pragma once

#include "RHI/Backend/Vulkan/Vulkan.hpp"
#include "RHI/Command.hpp"

namespace RHI
{

class Pass;

}

namespace Vulkan
{

class Context;
class PipelineState;

class RHI_EXPORT CommandList final
    : public RHI::CommandList
    , public DeviceObject<vk::CommandBuffer>
{
public:
    CommandList(vk::CommandBuffer commandBuffer)
        : DeviceObject(commandBuffer)
    {
    }

    ~CommandList() = default;

    void BeginRendering(RHI::Pass& passState);
    void EndRendering();

    void SetRenderArea(const RHI::DrawArea& region) override;

    void Submit(const RHI::Draw& cmd) override;
    void Submit(const RHI::Compute& cmd) override;
    void Submit(const RHI::Copy& cmd) override;

    void Begin() override;
    void End() override;

private:
    void BindShaderResourceGroups(const PipelineState& pipelineState, std::span<RHI::ShaderResourceGroup*> srgs);

private:
    const PipelineState* m_pipelineState;
};

}  // namespace Vulkan