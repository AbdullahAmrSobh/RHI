#pragma once
#include "RHI/Buffer.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"

namespace RHI
{
namespace Vulkan
{
    class Buffer final
        : public IBuffer
        , public DeviceObject<VkBuffer>
    {
		friend class Device;
		
    public:
        inline Buffer(Device& device)
            : DeviceObject(device)
        {
        }
        ~Buffer();

        VkResult Init(const MemoryAllocationDesc& allocDesc, const BufferDesc& desc);
        
		inline VmaAllocationInfo GetAllocationInfo() const { return m_allocationInfo; }		
		
    private:
        VmaAllocation     m_allocation = VK_NULL_HANDLE;
        VmaAllocationInfo m_allocationInfo;
		DeviceAddress     m_deviceAddress = nullptr;
    };

    class BufferView final
        : public IBufferView
        , public DeviceObject<VkBufferView>
    {
    public:
        inline BufferView(Device& device)
            : DeviceObject(device)
        {
        }
        ~BufferView();

        VkResult Init(const BufferViewDesc& desc);
    };

} // namespace Vulkan
} // namespace RHI
