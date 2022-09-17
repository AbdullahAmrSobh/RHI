#pragma once
#include <memory>
#include <vector>

#include "RHI/PipelineState.hpp"

#include <vulkan/vulkan.h>

#include "Backend/Vulkan/Resource.hpp"

namespace RHI
{
namespace Vulkan
{  
    class PipelineLayout final : public DeviceObject<VkPipelineLayout> 
    {
    public:
        ~PipelineLayout();
        
        VkResult Init(const PipelineLayoutDesc& layoutDesc);
        
        size_t GetHash();

    private:
        size_t m_hash;  
    };

    class PipelineState final
        : public IPipelineState
        , public DeviceObject<VkPipeline>
    {
    public:
        PipelineState(Device& device);
        ~PipelineState();
        
        VkResult Init(const GraphicsPipelineStateDesc& desc);
    
    private:
        std::shared_ptr<Internal::RenderPass> m_renderPass;
        std::shared_ptr<PipelineLayout> m_layout;
    };

} // namespace Vulkan
} // namespace RHI