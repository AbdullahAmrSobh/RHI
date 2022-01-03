#pragma once
#include "RHI/Texture.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{

    class Texture final
        : public ITexture
        , public DeviceObject<VkImage>
    {
    public:
        inline Texture(Device& device)
            : DeviceObject(device)
        {
        }
        ~Texture();

        VkResult Init(const MemoryAllocationDesc& allocDesc, const TextureDesc& desc);
        
        inline virtual size_t GetSize() const override { return m_allocationInfo.size; }
        
        inline virtual Expected<DeviceAddress> Map(size_t offset, size_t range) override
        {
            DeviceAddress deviceAddress;
            VkResult      result = vmaMapMemory(m_pDevice->GetAllocator(), m_allocation, &deviceAddress);
            if (result != VK_SUCCESS)
                return tl::unexpected(ToResultCode(result));
            return deviceAddress;
        }
        
        inline virtual void Unmap() override { vmaMapMemory(m_pDevice->GetAllocator(), m_allocation, &m_deviceAddress); }
    
    private:
        VmaAllocation     m_allocation = VK_NULL_HANDLE;
        VmaAllocationInfo m_allocationInfo;
		DeviceAddress     m_deviceAddress = nullptr;
    };

    class TextureView final
        : public ITextureView
        , public DeviceObject<VkImageView>
    {
    public:
        inline TextureView(Device& device)
            : DeviceObject(device)
        {
        }
        ~TextureView();

        VkResult Init(const TextureViewDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI
