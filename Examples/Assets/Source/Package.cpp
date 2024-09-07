#pragma once

#include "Assets/Package.hpp"

#include <TL/Serialization/Binary.hpp>

namespace Examples::Assets
{

    Package::Package() = default;

    Package::Package(const char* name, uint64_t uniqueID)
        : m_exportName(name)
        , m_uniqueID(uniqueID)
    {
    }

    void Package::AddSceneGraphs(TL::Span<const TL::String> locations)
    {
        for (auto location : locations)
        {
            m_sceneGraphs.push_back(location);
        }
    }

    void Package::AddMeshes(TL::Span<const TL::String> locations)
    {
        for (auto location : locations)
        {
            m_meshes.push_back(location);
        }
    }

    void Package::AddImages(TL::Span<const TL::String> locations)
    {
        for (auto location : locations)
        {
            m_images.push_back(location);
        }
    }

    void Package::AddMaterials(TL::Span<const TL::String> locations)
    {
        for (auto location : locations)
        {
            m_materials.push_back(location);
        }
    }

    TL::Span<const TL::String> Package::GetSceneGraphs() const
    {
        return m_sceneGraphs;
    }

    TL::Span<const TL::String> Package::GetMeshes() const
    {
        return m_meshes;
    }

    TL::Span<const TL::String> Package::GetImages() const
    {
        return m_images;
    }

    TL::Span<const TL::String> Package::GetMaterials() const
    {
        return m_materials;
    }
} // namespace Examples::Assets