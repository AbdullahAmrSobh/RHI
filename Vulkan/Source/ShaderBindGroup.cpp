#pragma once
#include "ShaderBindGroup.hpp"

namespace Vulkan
{

ShaderBindGroupAllocator* ShaderBindGroupAllocator::Create(Context* context)
{
    return {};
}

ShaderBindGroupAllocator::~ShaderBindGroupAllocator()
{
}

std::vector<RHI::Handle<RHI::ShaderBindGroup>> ShaderBindGroupAllocator::AllocateShaderBindGroups(RHI::TL::Span<RHI::ShaderBindGroupLayout> layouts)
{
    return {};
}

void ShaderBindGroupAllocator::Free(RHI::TL::Span<RHI::Handle<RHI::ShaderBindGroup>> groups)
{
}

void ShaderBindGroupAllocator::Update(RHI::Handle<RHI::ShaderBindGroup> group, const RHI::ShaderBindGroupData& data)
{
}

}  // namespace Vulkan