#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

class IDescriptorSet
{
public:
	virtual ~IDescriptorSet() = default;

};
using DescriptorSetPtr = Unique<IDescriptorSet>;

struct DescriptorsAllocatorDesc {};

class IDescriptorsAllocator
{
public:
    virtual ~IDescriptorsAllocator() = default;
	
	virtual DescriptorSetPtr AllocateDescriptorSet() = 0;
};
using DescriptorsAllocatorPtr = Unique<IDescriptorsAllocator>;

struct PipelineLayoutDesc
{
};

class IPipelineLayout
{
public:
    virtual ~IPipelineLayout() = default;
};
using PipelineLayoutPtr = Unique<IPipelineLayout>;

struct GraphicsPipelineStateDesc
{
};

struct ComputePipelineStateDesc
{
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;
};
using PipelineStatePtr = Unique<IPipelineState>;

} // namespace RHI
