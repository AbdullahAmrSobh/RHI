#pragma once

#include "Scene.hpp"

namespace Engine
{

    ResultCode Scene::Init(RHI::Device* device)
    {
        m_device = device;

        auto result = m_transforms.Init(*m_device, "g_transforms", RHI::BufferUsage::Vertex, 512);

        glm::mat4 scaler = glm::identity<glm::mat4>();
        scaler           = glm::scale(scaler, {4, 4, 4});
        auto t           = m_transforms.Insert(scaler);
        return result;
    }

    void Scene::Shutdown()
    {
        m_transforms.Shutdown();
    }

} // namespace Engine