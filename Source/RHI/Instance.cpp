#include "RHI/Pch.hpp"

#include "RHI/Common.hpp"

#include "Backend/Vulkan/Instance.hpp"

#include "Backend/Vulkan/Device.hpp"

namespace RHI
{

static Shared<IDebugCallbacks> s_callbacks;

void Debug::Init(Unique<IDebugCallbacks> callbacks)
{
    s_callbacks = std::move(callbacks);
}

Shared<IDebugCallbacks>& Debug::Get()
{
    return s_callbacks;
}

Expected<Unique<IInstance>> IInstance::Create(BackendType backend, Unique<IDebugCallbacks> callbacks)
{
    switch (backend)
    {
        case BackendType::Vulkan: {
            Unique<Vulkan::Instance> instance = CreateUnique<Vulkan::Instance>();

            Debug::Init(std::move(callbacks));

            VkResult result = instance->Init();

            if (result == VK_SUCCESS)
            {
                return instance;
            }

            return Unexpected(ResultCode::Fail);
        }
        default: return Unexpected(ResultCode::Fail);
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