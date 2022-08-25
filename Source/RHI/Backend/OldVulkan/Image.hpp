#pragma once
#include "RHI/Image.hpp"

#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/Semaphore.hpp"

namespace RHI
{
namespace Vulkan
{
    
    class SwapChain;

    class Image final
        : public IImage
        , public DeviceObject<VkImage>
    {
		friend class Device;
    
    public:
        Image(Device& device, VkImage image = VK_NULL_HANDLE, SwapChain* pSwapChain = nullptr)
            : DeviceObject(device, image)
            , m_pSwapChain(pSwapChain)
			, m_resourceIsReadySemaphore(device)
        {
        }
        ~Image();

        VkResult              Init(const MemoryAllocationDesc& allocDesc, const ImageDesc& desc);
		
		inline VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }		
		inline VkSemaphore GetResourceIsReadySemaphore() const { return m_resourceIsReadySemaphore.GetHandle(); }

    private:
        VmaAllocation     m_allocation = VK_NULL_HANDLE;
        VmaAllocationInfo m_allocationInfo;
        DeviceAddress     m_deviceAddress = nullptr;
        SwapChain*        m_pSwapChain    = nullptr;
		
		Semaphore m_resourceIsReadySemaphore;
	};
    
    class ImageView final
        : public IImageView
        , public DeviceObject<VkImageView>
    {
    public:
        ImageView(Device& device)
            : DeviceObject(device)
        {
        }
        ~ImageView();
        
        VkResult Init(const ImageViewDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI
