#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"
#include "RHI/PipelineState.hpp"
#include "RHI/Texture.hpp"

namespace RHI
{
namespace Vulkan
{
    class ShaderBytecode final
        : public IShaderBytecode
        , public DeviceObject<VkShaderModule>
    {
    public:
        inline ShaderBytecode(Device& device, const ShaderBytecodeDesc& desc)
            : DeviceObject(device)
            , IShaderBytecode(desc.entryPointName, desc.stage)
        {
        }

        ~ShaderBytecode();

        VkResult Init(const VkShaderModuleCreateInfo& createInfo);
    };

    namespace PipelineStateInitalizers
    {
        struct ShaderStage
        {
            ShaderStage(const IShaderBytecode* pVertex, const IShaderBytecode* pTessellationcontrol, const IShaderBytecode* pTessellationevaluation,
                        const IShaderBytecode* pGeometry, const IShaderBytecode* pFragment)
            {
                const ShaderBytecode& vertexShader                 = *static_cast<const ShaderBytecode*>(pVertex);
                const ShaderBytecode& tessellationControlShader    = *static_cast<const ShaderBytecode*>(pTessellationcontrol);
                const ShaderBytecode& tessellationEvaluationShader = *static_cast<const ShaderBytecode*>(pTessellationevaluation);
                const ShaderBytecode& geometryShader               = *static_cast<const ShaderBytecode*>(pGeometry);
                const ShaderBytecode& fragmentShader               = *static_cast<const ShaderBytecode*>(pFragment);

                VkPipelineShaderStageCreateInfo createInfo = {};
                createInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                createInfo.pNext                           = nullptr;
                createInfo.flags                           = 0;

                if (pVertex)
                {
                    createInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
                    createInfo.module              = vertexShader.GetHandle();
                    createInfo.pName               = pVertex->GetEntryPointName().c_str();
                    createInfo.pSpecializationInfo = nullptr;

                    states.push_back(createInfo);
                }

                if (pTessellationcontrol)
                {
                    createInfo.stage               = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    createInfo.module              = vertexShader.GetHandle();
                    createInfo.pName               = pTessellationcontrol->GetEntryPointName().c_str();
                    createInfo.pSpecializationInfo = nullptr;

                    states.push_back(createInfo);
                }

                if (pTessellationevaluation)
                {
                    createInfo.stage               = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    createInfo.module              = tessellationEvaluationShader.GetHandle();
                    createInfo.pName               = pTessellationevaluation->GetEntryPointName().c_str();
                    createInfo.pSpecializationInfo = nullptr;

                    states.push_back(createInfo);
                }

                if (pGeometry)
                {
                    createInfo.stage               = VK_SHADER_STAGE_GEOMETRY_BIT;
                    createInfo.module              = geometryShader.GetHandle();
                    createInfo.pName               = pGeometry->GetEntryPointName().c_str();
                    createInfo.pSpecializationInfo = nullptr;

                    states.push_back(createInfo);
                }

                if (pFragment)
                {

                    createInfo.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
                    createInfo.module              = fragmentShader.GetHandle();
                    createInfo.pName               = pFragment->GetEntryPointName().c_str();
                    createInfo.pSpecializationInfo = nullptr;

                    states.push_back(createInfo);
                }
            }

            inline void Initalize(uint32_t& count, VkPipelineShaderStageCreateInfo const*& pState)
            {
                count  = static_cast<uint32_t>(states.size());
                pState = states.data();
            }

            std::vector<VkPipelineShaderStageCreateInfo> states;
        };

        struct VertexInputState
        {
            explicit VertexInputState(const std::vector<EVertexAttributeFormat>& formats)
            {
                bindingDescription.stride = 0;

                uint32_t location = 0;

                for (auto format : formats)
                {
                    VkVertexInputAttributeDescription attribute = {};
                    attribute.binding                           = 0;
                    attribute.location                          = location;
                    attribute.format                            = Utils::ConvertBufferFormat(format);
                    attribute.offset                            = bindingDescription.stride;

                    bindingDescription.stride += Utils::GetFormatTaxelSize(attribute.format);

                    location++;
                    attributes.push_back(attribute);
                }

                bindingDescription.binding   = 0;
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                state                                 = {};
                state.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                state.pNext                           = nullptr;
                state.flags                           = 0;
                state.vertexBindingDescriptionCount   = 1;
                state.pVertexBindingDescriptions      = &bindingDescription;
                state.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
                state.pVertexAttributeDescriptions    = attributes.data();
            }

            inline void Initalize(VkPipelineVertexInputStateCreateInfo const*& pState) const { pState = &state; }

            VkVertexInputBindingDescription                bindingDescription;
            std::vector<VkVertexInputAttributeDescription> attributes;
            VkPipelineVertexInputStateCreateInfo           state;
        };

        struct InputAssemblyState
        {
            explicit InputAssemblyState()
            {
                state.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                state.pNext                  = nullptr;
                state.flags                  = 0;
                state.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                state.primitiveRestartEnable = VK_FALSE;
            }

            inline void Initalize(VkPipelineInputAssemblyStateCreateInfo const*& pState) const { pState = &state; }

            VkPipelineInputAssemblyStateCreateInfo state;
        };

        struct TessellationState
        {
            explicit TessellationState()
            {

                state.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
                state.pNext              = nullptr;
                state.flags              = 0;
                state.patchControlPoints = 0;
            }

            inline void Initalize(VkPipelineTessellationStateCreateInfo const*& pState) const { pState = &state; }

            VkPipelineTessellationStateCreateInfo state;
        };

        struct ViewportState
        {
            explicit ViewportState()
            {
                state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                state.pNext         = nullptr;
                state.flags         = 0;
                state.viewportCount = 0;
                state.pViewports    = nullptr;
                state.scissorCount  = 0;
                state.pScissors     = nullptr;
            }

            inline void Initalize(VkPipelineViewportStateCreateInfo const*& pState) const { pState = &state; }

            VkPipelineViewportStateCreateInfo state;
        };

        struct RasterizationState
        {
            explicit RasterizationState()
            {
                state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                state.pNext = nullptr;

                state.depthClampEnable = VK_FALSE;
                // discards all primitives before the rasterization stage if enabled which we don't want
                state.rasterizerDiscardEnable = VK_FALSE;

                state.polygonMode = VK_POLYGON_MODE_FILL;
                state.lineWidth   = 1.0f;
                // no backface cull
                state.cullMode  = VK_CULL_MODE_NONE;
                state.frontFace = VK_FRONT_FACE_CLOCKWISE;
                // no depth bias
                state.depthBiasEnable         = VK_FALSE;
                state.depthBiasConstantFactor = 0.0f;
                state.depthBiasClamp          = 0.0f;
                state.depthBiasSlopeFactor    = 0.0f;
            }

            inline void Initalize(VkPipelineRasterizationStateCreateInfo const*& pState) const { pState = &state; }

            VkPipelineRasterizationStateCreateInfo state;
        };

        struct MultisampleState
        {
            explicit MultisampleState(ESampleCount sampleCount)
            {
                state.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                state.pNext                 = nullptr;
                state.flags                 = 0;
                state.rasterizationSamples  = Utils::ConvertSampleCount(sampleCount);
                state.sampleShadingEnable   = VK_FALSE;
                state.minSampleShading      = 1.0f;
                state.pSampleMask           = nullptr;
                state.alphaToCoverageEnable = VK_FALSE;
                state.alphaToOneEnable      = VK_FALSE;
            }

            inline void Initalize(VkPipelineMultisampleStateCreateInfo const*& pState) const { pState = &state; }

            VkPipelineMultisampleStateCreateInfo state;
        };

        struct DepthStencilState
        {
            explicit DepthStencilState(bool depthEnable = false, bool stencilEnable = false)
            {
                state.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                state.pNext                 = nullptr;
                state.flags                 = 0;
                state.depthTestEnable       = depthEnable ? VK_TRUE : VK_FALSE;
                state.depthWriteEnable      = VK_TRUE;
                state.depthCompareOp        = VK_COMPARE_OP_LESS;
                state.depthBoundsTestEnable = VK_FALSE;
                state.stencilTestEnable     = stencilEnable ? VK_TRUE : VK_FALSE;
                state.front                 = {};
                state.back                  = {};
                state.minDepthBounds        = 0.0f;
                state.maxDepthBounds        = 1.0f;
            }

            inline void Initalize(VkPipelineDepthStencilStateCreateInfo const*& pState) const { pState = &state; }

            VkPipelineDepthStencilStateCreateInfo state;
        };

        struct ColorBlendState
        {
            explicit ColorBlendState()
            {
                state.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                state.pNext             = nullptr;
                state.flags             = 0;
                state.logicOpEnable     = VK_FALSE;
                state.logicOp           = VK_LOGIC_OP_SET;
                state.attachmentCount   = attachmentColorBlend.size();
                state.pAttachments      = attachmentColorBlend.data();
                state.blendConstants[0] = 0.0f;
                state.blendConstants[1] = 0.0f;
                state.blendConstants[2] = 0.0f;
                state.blendConstants[3] = 0.0f;
            }

            inline void Initalize(VkPipelineColorBlendStateCreateInfo const*& pState) const { pState = &state; }

            std::vector<VkPipelineColorBlendAttachmentState> attachmentColorBlend;
            VkPipelineColorBlendStateCreateInfo              state;
        };

        struct DynamicState
        {
            explicit DynamicState()
            {
                VkDynamicState dynamicStates[] = {
                    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
                    // VK_DYNAMIC_STATE_LINE_WIDTH,
                    // VK_DYNAMIC_STATE_DEPTH_BIAS,
                    // VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                    // VK_DYNAMIC_STATE_DEPTH_BOUNDS,
                    // VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
                    // VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                    // VK_DYNAMIC_STATE_STENCIL_REFERENCE,
                };
                state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                state.pNext             = nullptr;
                state.flags             = 0;
                state.dynamicStateCount = 2;
                state.pDynamicStates    = dynamicStates;
            }

            inline void Initalize(VkPipelineDynamicStateCreateInfo const*& pState) const { pState = &state; }

            VkPipelineDynamicStateCreateInfo state;
        };

    } // namespace PipelineStateInitalizers

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
