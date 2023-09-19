#pragma once
#include <RHI/ShaderBindGroup.hpp>

namespace Vulkan
{

class Context;

struct DescriptorSet : RHI::ShaderBindGroup
{
    struct Descriptor
    {
    };
};

class ShaderBindGroupAllocator final : public RHI::ShaderBindGroupAllocator
{
public:
    static ShaderBindGroupAllocator* Create(Context* context);

    using RHI::ShaderBindGroupAllocator::ShaderBindGroupAllocator;
    ~ShaderBindGroupAllocator();

    std::vector<RHI::Handle<RHI::ShaderBindGroup>> AllocateShaderBindGroups(RHI::TL::Span<RHI::ShaderBindGroupLayout> layouts) override;

    void Free(RHI::TL::Span<RHI::Handle<RHI::ShaderBindGroup>> groups) override;

    void Update(RHI::Handle<RHI::ShaderBindGroup> group, const RHI::ShaderBindGroupData& data) override;
};

}  // namespace Vulkan