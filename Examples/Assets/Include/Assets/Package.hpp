#pragma once

#include "Assets/Name.hpp"
#include "Assets/Mesh.hpp"
#include "Assets/Material.hpp"
#include "Assets/SceneGraph.hpp"
#include "Assets/Image.hpp"

#include <TL/Memory.hpp>
#include <TL/Containers.hpp>

#include <filesystem>

namespace Examples::Assets
{

    class Package
    {
    public:
        Package() = default;

        Package(const char* name)
            : m_name(name)
        {
        }

        inline void AddSceneGraph(std::filesystem::path path) { m_sceneGraphs.push_back(TL::String(path.string())); }

        inline void AddSceneGraphs(TL::Span<const std::filesystem::path> paths)
        {
            for (auto path : paths)
            {
                AddSceneGraph(path);
            }
        }

        inline void AddMesh(std::filesystem::path path) { m_meshes.push_back(TL::String(path.string())); }

        inline void AddMeshs(TL::Span<const std::filesystem::path> paths)
        {
            for (auto path : paths)
            {
                AddMesh(path);
            }
        }

        inline void AddImage(std::filesystem::path path) { m_images.push_back(TL::String(path.string())); }

        inline void AddImages(TL::Span<const std::filesystem::path> paths)
        {
            for (auto path : paths)
            {
                AddImage(path);
            }
        }

        inline void AddMaterial(std::filesystem::path path) { m_materials.push_back(TL::String(path.string())); }

        inline void AddMaterials(TL::Span<const std::filesystem::path> paths)
        {
            for (auto path : paths)
            {
                AddMaterial(path);
            }
        }

        inline TL::Span<const TL::String> GetSceneGraphs(std::filesystem::path path) const { return m_sceneGraphs; }

        inline TL::Span<const TL::String> GetMeshs(std::filesystem::path path) const { return m_meshes; }

        inline TL::Span<const TL::String> GetImages(std::filesystem::path path) const { return m_images; }

        inline TL::Span<const TL::String> GetMaterials(std::filesystem::path path) const { return m_materials; }

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_name);
            TL::Encode(archive, m_sceneGraphs);
            TL::Encode(archive, m_meshes);
            TL::Encode(archive, m_images);
            TL::Encode(archive, m_materials);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_name);
            TL::Decode(archive, m_sceneGraphs);
            TL::Decode(archive, m_meshes);
            TL::Decode(archive, m_images);
            TL::Decode(archive, m_materials);
        }

    private:
        TL::String m_name;

        TL::Vector<TL::String> m_sceneGraphs;
        TL::Vector<TL::String> m_meshes;
        TL::Vector<TL::String> m_images;
        TL::Vector<TL::String> m_materials;

        TL::UnorderedMap<Name, TL::WeakRef<SceneGraph>> m_sceneGraphsRefs;
        TL::UnorderedMap<Name, TL::WeakRef<Mesh>> m_meshesRefs;
        TL::UnorderedMap<Name, TL::WeakRef<Image>> m_imagesRefs;
        TL::UnorderedMap<Name, TL::WeakRef<Material>> m_materialRefs;
    };
} // namespace Examples::Assets