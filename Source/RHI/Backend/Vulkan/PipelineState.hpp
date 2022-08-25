#pragma once
#include "RHI/Backend/Vulkan/Resource.hpp"
#include "RHI/PipelineState.hpp"

namespace RHI
{
namespace Vulkan
{

    class GraphicsPipelineStateCreateInfoInitializer
    {
    public:
        GraphicsPipelineStateCreateInfoInitializer(const GraphicsPipelineStateDesc& desc);

        void Initalize(VkGraphicsPipelineStateCreateInfo* pCreateInfo) const;

    private:
        std::vector<const VkPipelineShaderStageCreateInfo> stages;
        
        struct VertexInputState 
        {
            VkPipelineVertexInputStateCreateInfo createInfo;
        } m_vertexInputState;
        
        VkPipelineInputAssemblyStateCreateInfo             inputAssemblyState;
        VkPipelineTessellationStateCreateInfo              tessellationState;
        VkPipelineViewportStateCreateInfo                  viewportState;
        VkPipelineRasterizationStateCreateInfo             rasterizationState;
        VkPipelineMultisampleStateCreateInfo               multisampleState;
        VkPipelineDepthStencilStateCreateInfo              depthStencilState;
        VkPipelineColorBlendStateCreateInfo                colorBlendState;
        VkPipelineDynamicStateCreateInfo                   dynamicState;
        VkPipelineLayout                                   layout;
        VkRenderPass                                       renderPass;
        VkGraphicsPipelineStateCreateInfo                  m_createInfo;
    };

    class PipelineState final
        : public IPipelineState
        , public Resource<IPipelineState>
    {
    public:
        PipelineState(Device& device);
        ~PipelineState();

        VkResult Init(const GraphicsPipelineStateDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI