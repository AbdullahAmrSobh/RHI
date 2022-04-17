#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

class ISampler;

struct Descriptor
{
    Descriptor() = default;

    inline Descriptor(EDescriptorType type, EDescriptorAccessType access, uint32_t count, ShaderStageFlags bindStages)
        : type(type)
        , accessType(access)
        , count(count)
        , stages(bindStages)
    {
    }
    
    inline Descriptor(const ISampler& immutableSampler, ShaderStageFlags bindStages, uint32_t count = 1)
        : type(EDescriptorType::Sampler)
        , accessType(EDescriptorAccessType::Undefined)
        , count(count)
        , stages(bindStages)
        , pImmutableSampler(&immutableSampler)
    {
    }
    
    EDescriptorType       type;
    EDescriptorAccessType accessType;
    uint32_t              count;
    ShaderStageFlags      stages;
    const ISampler*       pImmutableSampler;
};

struct DescriptorSeLayoutDesc
{
    DescriptorSeLayoutDesc() = default;
    DescriptorSeLayoutDesc(std::initializer_list<Descriptor> descriptors)
        : m_descriptors(descriptors)
    {
    }

    inline void AddDescriptor(const Descriptor& descriptor)
    {
        m_descriptors.push_back(descriptor);
    }

    std::vector<Descriptor> m_descriptors;
};

class IDescriptorSetLayout
{
public:
    virtual ~IDescriptorSetLayout() = default;
};
using DescriptorSetLayoutPtr = Unique<IDescriptorSetLayout>;

} // namespace RHI
