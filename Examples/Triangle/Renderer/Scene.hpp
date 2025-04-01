#pragma once

#include <RHI/RHI.hpp>

#include <TL/Containers.hpp>
#include <TL/Span.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "BufferPool.hpp"
#include "Common.hpp"

namespace Engine
{
    class Scene
    {
    public:
        Scene()  = default;
        ~Scene() = default;

        ResultCode Init(RHI::Device* device);
        void       Shutdown();

        RHI::BufferBindingInfo GetTransformsInstanceBuffer() const { return m_transforms.GetBindingInfo(); }

    private:
        RHI::Device*        m_device;
        GpuArray<glm::mat4> m_transforms;
    };
} // namespace Engine