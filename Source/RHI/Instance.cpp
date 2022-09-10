#include <cassert>
#include <utility>

#include "RHI/Common.hpp"
#include "RHI/Instance.hpp"

#include <vulkan/vulkan.h>

#include "Backend/Vulkan/Device.hpp"
#include "Backend/Vulkan/Instance.hpp"

namespace RHI
{

Expected<Unique<IInstance>> IInstance::Create(EBackend backend, Unique<IDebugCallbacks> callbacks)
{
    switch (backend)
    {
    case EBackend::Vulkan:
    {
        Unique<Vulkan::Instance> instance = CreateUnique<Vulkan::Instance>();

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

} // namespace RHI