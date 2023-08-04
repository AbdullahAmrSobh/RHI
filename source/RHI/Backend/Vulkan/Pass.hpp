#pragma once

#include <vector>

#include "RHI/Pass.hpp"

#include "RHI/Backend/Vulkan/Vulkan.hpp"

#include <vulkan/vulkan.hpp>

namespace Vulkan
{

enum class BarrierType
{
    GraphicsBegin,
    GraphicsEnd,
    ComputeBegin,
    ComputeEnd,
    TransferBegin,
    TransferEnd,
};

struct Barrier
{
    BarrierType type;

    union
    {
        vk::MemoryBarrier2       memoryBarrier;
        vk::ImageMemoryBarrier2  imageBarrier;
        vk::BufferMemoryBarrier2 bufferBarrier;
    };

    VkDependencyInfo barrierInfo;
};

class Pass final : public RHI::Pass
{
    friend class FrameScheduler;

public:
    inline vk::Semaphore GetSemaphore() const;

    inline std::span<const Barrier> GetBarriers() const;

private:
    std::vector<Barrier> m_barriers;
    vk::UniqueSemaphore  m_semaphore;
};

}  // namespace Vulkan