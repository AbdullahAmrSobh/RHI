#pragma once
#include <memory>
#include <vector>

#include "RHI/PipelineState.hpp"

#include <vulkan/vulkan.h>

#include "Backend/Vulkan/Resource.hpp"
#include <vulkan/vulkan_core.h>

namespace RHI
{
namespace Vulkan
{  
    class GraphicsPipelineStateCreateInfoInitializer
    {
    public:
        GraphicsPipelineStateCreateInfoInitializer(const GraphicsPipelineStateDesc& desc);

    private:
        std::vector<VkPipelineShaderStageCreateInfo>     m_shaderStages;
        std::vector<VkVertexInputBindingDescription>     m_vertexBindings;
        std::vector<VkVertexInputAttributeDescription>   m_vertexAttributes;
        VkPipelineVertexInputStateCreateInfo             m_vertexInputState;
        VkPipelineInputAssemblyStateCreateInfo           m_inputAssemblyState;
        VkPipelineTessellationStateCreateInfo            m_tessellationState;
        // VkPipelineViewportStateCreateInfo             m_viewportState;
        VkPipelineRasterizationStateCreateInfo           m_rasterizationState;
        VkPipelineMultisampleStateCreateInfo             m_multisampleState;
        VkPipelineDepthStencilStateCreateInfo            m_depthStencilState;
        std::vector<VkPipelineColorBlendAttachmentState> m_colorBlendAttachments;
        VkPipelineColorBlendStateCreateInfo              m_colorBlendState;
        std::vector<VkDynamicState>                      m_dynamicStates;
        VkPipelineDynamicStateCreateInfo                 m_dynamicState;
        VkPipelineLayout                                 m_layout;
        VkRenderPass                                     m_renderPass;
        VkGraphicsPipelineCreateInfo                     m_createInfo;
    };

    class PipelineLayout final : public Resource<VkPipelineLayout> 
    {
    public:
        VkResult Init(const PipelineLayoutDesc& layoutDesc);
        
        size_t GetHash();

    private:
        size_t m_hash;  
    };

    class PipelineState final
        : public IPipelineState
        , public Resource<IPipelineState>
    {
    public:
        PipelineState(Device& device);
        ~PipelineState();

        VkResult Init(const GraphicsPipelineStateDesc& desc);
    
    private:
        std::shared_ptr<PipelineLayout> m_layout;
    };

} // namespace Vulkan
} // namespace RHI