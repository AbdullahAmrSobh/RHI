#include "RHI/Backend/Vulkan/Buffer.hpp"
#include "RHI/Backend/Vulkan/Factory.hpp"
#include "RHI/Backend/Vulkan/Utils.hpp"

namespace RHI
{
namespace Vulkan
{
    Expected<BufferPtr> Factory::CreateBuffer(const MemoryAllocationDesc& _allocDesc, const BufferDesc& _desc)
    {
        // Create a buffer resource.
        auto     buffer = CreateUnique<Buffer>(*m_device);
        VkResult result = buffer->Init(_allocDesc, _desc);

        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return buffer;
    }

    Buffer::~Buffer() { vmaDestroyBuffer(m_pDevice->GetAllocator(), m_handle, m_allocation); }

    VkResult Buffer::Init(const MemoryAllocationDesc& allocDesc, const BufferDesc& desc)
    {
        VkBufferCreateInfo createInfo = {};
        createInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext              = nullptr;
        createInfo.flags              = 0;
        createInfo.size               = desc.size;
        memcpy(&createInfo.usage, &desc.usage, sizeof(desc.usage));
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
        
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage                   = static_cast<VmaMemoryUsage>(allocDesc.usage);
        
        return vmaCreateBuffer(m_pDevice->GetAllocator(), &createInfo, &allocInfo, &m_handle, &m_allocation, &m_allocationInfo);
    }

    Expected<BufferViewPtr> Factory::CreateBufferView(const BufferViewDesc& desc)
    {
        auto     bufferView = CreateUnique<BufferView>(*m_device);
        VkResult result     = bufferView->Init(desc);

        if (result != VK_SUCCESS)
            return tl::unexpected(ToResultCode(result));

        return bufferView;
    }

    BufferView::~BufferView() { vkDestroyBufferView(m_pDevice->GetHandle(), m_handle, nullptr); }

    VkResult BufferView::Init(const BufferViewDesc& desc)
    {
        VkBufferViewCreateInfo createInfo = {};
        createInfo.sType                  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                  = nullptr;
        createInfo.flags                  = 0;
        createInfo.buffer                 = static_cast<Buffer&>(*desc.pBuffer).GetHandle();
        createInfo.format                 = Utils::ConvertBufferFormat(desc.format);
        createInfo.offset                 = desc.offset;
        createInfo.range                  = desc.size;

        return vkCreateBufferView(m_pDevice->GetHandle(), &createInfo, nullptr, &m_handle);
    }

} // namespace Vulkan
} // namespace RHI
