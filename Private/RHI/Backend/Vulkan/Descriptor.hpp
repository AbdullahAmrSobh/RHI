#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Descriptor.hpp"

namespace RHI
{
namespace Vulkan
{
    extern std::unordered_map<size_t, VkDescriptorSetLayout> g_descriptorSetLayoutMap;
    class DescriptorSet                                      final
        : public IDescriptorSet
        , public DeviceObject<VkDescriptorSet>
    {
    public:
        inline DescriptorSet(Device& device, VkDescriptorPool paranetPool)
            : DeviceObject(device)
            , m_parantPool(paranetPool)
        {
        }
        ~DescriptorSet();

        VkResult Init(const DescriptorSetLayout& layout);

        virtual void BeginUpdate() override;
        virtual void EndUpdate() override;
        virtual void BindResource(uint32_t dstBinding, ITexture& texture) override;
        virtual void BindResource(uint32_t dstBinding, ArrayView<ITexture*> textures) override;
        virtual void BindResource(uint32_t dstBinding, IBuffer& buffer) override;
        virtual void BindResource(uint32_t dstBinding, ArrayView<IBuffer*> buffers) override;

    private:
        VkDescriptorPool                  m_parantPool = VK_NULL_HANDLE;
        std::vector<VkCopyDescriptorSet>  m_copyInfo;
        std::vector<VkWriteDescriptorSet> m_writeInfo;
        bool                              m_isUpdateing = false;
    };

    class DescriptorPool final
        : public IDescriptorPool
        , public DeviceObject<VkDescriptorPool>
    {
    public:
        inline DescriptorPool(Device& device)
            : DeviceObject(device)
        {
        }
        ~DescriptorPool();

        VkResult Init(const DescriptorPoolDesc& desc);

        virtual Expected<DescriptorSetPtr> AllocateDescriptorSet(const DescriptorSetLayout& layout) override;
    };

} // namespace Vulkan
} // namespace RHI
