#pragma once
#include "RHI/PipelineState.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include "Backend/Vulkan/ShaderResourceGroup.hpp"

namespace RHI
{
namespace Vulkan
{
    class PipelineLayout final : public DeviceObject<VkPipelineLayout>
    {
    public:
        static Result<Unique<PipelineLayout>> Create(const Device& device, PipelineLayoutDesc& layoutDesc);

        PipelineLayout(const Device& device)
            : DeviceObject(&device)
        {
        }
        
        ~PipelineLayout();
        
        VkResult Init(const PipelineLayoutDesc& layoutDesc);

        inline size_t GetHash() const
        {
            return m_hash;
        }

        inline const std::vector<Unique<DescriptorSetLayout>>& GetDescriptorLayouts() const
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
        
        inline const PipelineLayout& GetLayout() const
        {
            return *m_layout;
        }

    private:
        Shared<PipelineLayout> m_layout;
    };

} // namespace Vulkan
} // namespace RHI