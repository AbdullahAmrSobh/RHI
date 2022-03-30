#pragma once
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Definitions.hpp"

namespace RHI
{
namespace Vulkan
{
    class RenderPass : public DeviceObject<VkRenderPass>
    {
    private:
        friend class Factory;

    public:
        RenderPass(Device& device)
            : DeviceObject(device)
        {
        }
        ~RenderPass();

        VkResult Init(const class RenderPassBuilder& builder);

        uint32_t GetSubpassCount() const;
        uint32_t GetSubpassIndex(const struct PassId id) const;

        static RenderPass& FindOrCreate(Device& device, const struct PipelineStateRenderTargetLayout& layout);
    private:
        static std::unordered_map<uint64_t, RenderPass*> s_cache;
    };
    using RenderPassRef = Shared<RenderPass>;

} // namespace Vulkan
} // namespace RHI
