#pragma once

#include <vulkan/vulkan.h>

namespace Vulkan
{

struct PipelineLayout
{
    struct Descriptor
    {
    };

    VkPipelineLayout layout;
};

struct GraphicsPipeline
{
    struct Descriptor
    {
    };

    VkPipeline pipeline;
};

struct ComputePipeline
{
    struct Descriptor
    {
    };

    VkPipeline pipeline;
};

struct Sampler
{
    struct Descriptor
    {
    };

    VkSampler sampler;
};

}  // namespace Vulkan