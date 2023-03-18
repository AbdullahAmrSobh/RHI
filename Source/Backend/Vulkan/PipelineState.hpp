#pragma once
#include "RHI/PipelineState.hpp"

#include "Backend/Vulkan/DeviceObject.hpp"
#include "Backend/Vulkan/ShaderResourceGroup.hpp"
#include "Backend/Vulkan/Framebuffer.hpp"

namespace RHI
{
namespace Vulkan
{
class DescriptorSetLayout;

class PipelineLayout final : public DeviceObject<VkPipelineLayout>
{
public:
    PipelineLayout(Device& device)
        : DeviceObject(device)
    {
    }

    ~PipelineLayout();

    VkResult Init(const PipelineLayoutDesc& layoutDesc);

    size_t GetHash() const
    {
        return m_hash;
    }

private:
    size_t                                   m_hash;
    std::vector<VkPushConstantRange>         m_pushConstantRanges;
    std::vector<std::unique_ptr<DescriptorSetLayout>> m_descriptorSetsLayouts;
};

VkShaderStageFlags CovnertShaderStages(ShaderStageFlags stages);

VkCullModeFlags ConvertRasterizationStateCullMode(RasterizationCullMode cullMode);
VkPolygonMode   ConvertRasterizationStateFillMode(RasterizationFillMode fillMode);

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

    const PipelineLayout& GetLayout() const
    {
        return *m_layout;
    }

private:
    std::unique_ptr<PipelineLayout>     m_layout;
    std::shared_ptr<RenderPassLayout> m_renderTargetLayout;
};

}  // namespace Vulkan
}  // namespace RHI