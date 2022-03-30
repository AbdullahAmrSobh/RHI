#pragma once
#include "RHI/Texture.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Semaphore.hpp"

namespace RHI
{
namespace Vulkan
{
    
    class SwapChain;

    class Texture final
        : public ITexture
        , public DeviceObject<VkImage>
    {
		friend class Device;
    
    public:
        inline Texture(Device& device, VkImage image = VK_NULL_HANDLE, SwapChain* pSwapChain = nullptr)
            : DeviceObject(device, image)
            , m_pSwapChain(pSwapChain)
			, m_resourceIsReadySemaphore(device)
        {
        }
        ~Texture();

        VkResult              Init(const MemoryAllocationDesc& allocDesc, const TextureDesc& desc);
		
		inline VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }		
		inline VkSemaphore GetResourceIsReadySemaphore() const { return m_resourceIsReadySemaphore.GetHandle(); }

		virtual size_t GetSize() const override { return m_allocationInfo.size; }

    private:
        VmaAllocation     m_allocation = VK_NULL_HANDLE;
        VmaAllocationInfo m_allocationInfo;
        DeviceAddress     m_deviceAddress = nullptr;
        SwapChain*        m_pSwapChain    = nullptr;
		
		Semaphore m_resourceIsReadySemaphore;
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
