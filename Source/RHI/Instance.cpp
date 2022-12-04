#include "Backend/Vulkan/Instance.hpp"

#include "Backend/Vulkan/Device.hpp"
#include "RHI/Common.hpp"

namespace RHI
{

Expected<Unique<IInstance>> IInstance::Create(EBackend                backend,
                                              Unique<IDebugCallbacks> callbacks)
{
    switch (backend)
    {
        case EBackend::Vulkan: {
            Unique<Vulkan::Instance> instance =
                CreateUnique<Vulkan::Instance>();

            instance->m_debugCallbacks = std::move(callbacks);

            VkResult result = instance->Init();

            if (result == VK_SUCCESS)
            {
                return instance;
            }

            return Unexpected(EResultCode::Fail);
        }
        default: return Unexpected(EResultCode::Fail);
    }
}

std::vector<const IPhysicalDevice*> IInstance::GetPhysicalDevices() const
{
    std::vector<const IPhysicalDevice*> result;
    for (auto& physicalDevice : m_physicalDevices)
    {
        result.push_back(physicalDevice.get());
    }
    return result;
}

}  // namespace RHI