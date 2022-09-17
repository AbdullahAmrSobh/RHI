#pragma once
#include "RHI/ShaderResourceGroup.hpp"
#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Resource.hpp"
#include <vulkan/vulkan.h>

namespace RHI
{
namespace Vulkan
{
    
    class ShaderResourceGroup final : public IShaderResourceGroup
    {
    public:
        ShaderResourceGroup(const Device& device); 
        ~ShaderResourceGroup();

        virtual EResultCode Update(const ShaderResourceGroupData& data) override;
    };

    class ShaderResourceGroupAllocator final : public IShaderResourceGroupAllocator
    {
    public:
        ShaderResourceGroupAllocator(const Device& device);
        ~ShaderResourceGroupAllocator();
        
        virtual Expected<Unique<IShaderResourceGroup>> Allocate(const ShaderResourceGroupLayout& layout) const override;
        
        virtual void Free(Unique<IShaderResourceGroup> group) override;
    };

} // namespace Vulkan
} // namespace RHI