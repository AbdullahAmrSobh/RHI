#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/DescriptorSet.hpp"

namespace RHI
{

class IDescriptorSetLayout;

struct DescriptorSize
{
    DescriptorSize() = default;
    DescriptorSize(EDescriptorType type, uint32_t count = 1)
        : count(count)
        , type(type)
    {
    }

    EDescriptorType type;
    uint32_t        count;
};

struct DescriptorPoolDesc
{
    inline void AddDescriptor(EDescriptorType type, uint32_t count = 1)
    {
        descriptors.emplace_back(type, count);
    }

    std::vector<DescriptorSize> descriptors;
    uint32_t                    maxSets;
};

class IDescriptorPool
{
public:
    virtual ~IDescriptorPool() = default;

    virtual void             Reset()                                      = 0;
    virtual DescriptorSetPtr Allocate(const IDescriptorSetLayout& layout) = 0;
};
using DescriptorPoolPtr = Unique<IDescriptorPool>;

} // namespace RHI
