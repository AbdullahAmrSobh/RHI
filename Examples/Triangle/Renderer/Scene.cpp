#pragma once

#include "Scene.hpp"

namespace Engine
{

    ResultCode Scene::Init(RHI::Device* device)
    {
        m_device = device;

        return m_transforms.Init(*m_device, "g_transforms", RHI::BufferUsage::Vertex, 512);
    }

    void Scene::Shutdown()
    {
        m_transforms.Shutdown();
    }

} // namespace Engine