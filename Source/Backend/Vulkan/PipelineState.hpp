#pragma once
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/ShaderResourceGroup.hpp"
#include "RHI/PipelineState.hpp"

namespace RHI
{
namespace Vulkan
{
class DescriptorSetLayout;

class PipelineLayout final : public DeviceObject<VkPipelineLayout>
{
public:
    PipelineLayout(const Device& device)
        : DeviceObject(&device)
    {
    }

    ~PipelineLayout();

    VkResult Init(const PipelineLayoutDesc& layoutDesc);

    size_t GetHash() const
    {
        return m_hash;
    }

    const std::vector<Unique<DescriptorSetLayout>>& GetDescriptorLayouts() const
    {
        return m_descriptorSetsLayouts;
    }

private:
    size_t                                   m_hash;
    std::vector<VkPushConstantRange>         m_pushConstantRanges;
    std::vector<Unique<DescriptorSetLayout>> m_descriptorSetsLayouts;
};

class PipelineState final
    : public IPipelineState
    , public DeviceObject<VkPipeline>
{
public:
    PipelineState(const Device& device)
        : DeviceObject(&device)
    {
    }

    ~PipelineState();

    VkResult Init(const GraphicsPipelineStateDesc& desc);

    const PipelineLayout& GetLayout() const
    {
        return *m_layout;
    }

private:
    Unique<PipelineLayout> m_layout;
};

}  // namespace Vulkan
}  // namespace RHI