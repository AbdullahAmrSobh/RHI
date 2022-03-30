#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/DescriptorSet.hpp"

namespace RHI
{

class IDescriptorSetLayout;

struct DescriptorPoolDesc
{

    struct DescriptorSize
    {
        EDescriptorType type;
        uint32_t        count;
    };

    inline DescriptorPoolDesc& AddDescriptor(EDescriptorType type, uint32_t count = 1)
    {
        descriptors.emplace_back(type, count);
        return *this;
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
