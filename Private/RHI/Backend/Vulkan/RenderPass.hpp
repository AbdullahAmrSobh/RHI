#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Backend/Vulkan/Device.hpp"
#include "RHI/Backend/Vulkan/RenderTarget.hpp"

namespace RHI
{
namespace Vulkan
{
    class RenderPass : public DeviceObject<VkRenderPass>
    {
    private:
        friend class Factory;
        static void InitCacheManager(Device& device);
    
    public:
        RenderPass(Device& device)
            : DeviceObject(device)
        {
        }
        ~RenderPass();
        
        VkResult Init(const RenderTargetDesc& desc);

        static RenderPass& FindOrCreate(const RenderTargetDesc& desc);
    
    private:
		
        std::vector<VkAttachmentDescription> SetupColorAttachments(const RenderTargetDesc& desc);
    
    private:
        static Device*                                        s_pDevice;
        static std::unordered_map<size_t, Unique<RenderPass>> s_RenderPasses;
    };
    using RenderPassRef = Shared<RenderPass>;

} // namespace Vulkan
} // namespace RHI
