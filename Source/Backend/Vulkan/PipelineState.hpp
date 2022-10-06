#pragma once
#include "RHI/PipelineState.hpp"
#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{
    class DescriptorSetLayout;
    
    class PipelineLayout final : public DeviceObject<VkPipelineLayout>
    {
    public:
        ~PipelineLayout();

        VkResult Init(const PipelineLayoutDesc& layoutDesc);

        size_t GetHash() const
        {
            return m_hash;
        }

        inline std::vector<Unique<DescriptorSetLayout>> GetDescriptorLayouts() const
        {
            return m_descriptorSetsLayouts;
        }

    private:
        size_t                                   m_hash;
        std::vector<Unique<DescriptorSetLayout>> m_descriptorSetsLayouts;
    };

    class PipelineState final
        : public IPipelineState
        , public DeviceObject<VkPipeline>
    {
    public:
        PipelineState(Device& device);
        ~PipelineState();

        VkResult Init(const GraphicsPipelineStateDesc& desc);

        inline const PipelineLayout& GetLayout() const
        {
            return *m_layout;
        }

    private:
        // std::shared_ptr<Internal::RenderPass> m_renderPass;
        std::shared_ptr<PipelineLayout> m_layout;
    };

} // namespace Vulkan
} // namespace RHI