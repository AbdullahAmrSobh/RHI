#pragma once

#include <TL/Memory.hpp>
#include <TL/Containers.hpp>
#include <TL/Serialization/Binary.hpp>

namespace Examples::Assets
{
    class Package
    {
    public:
        Package();
        Package(const char* name, uint64_t uniqueID);

        inline const TL::String& GetName() const { return m_exportName; }

        inline uint64_t GetID() const { return m_uniqueID; }

        void AddSceneGraphs(TL::Span<const TL::String> locations);

        void AddMeshes(TL::Span<const TL::String> locations);

        void AddImages(TL::Span<const TL::String> locations);

        void AddMaterials(TL::Span<const TL::String> locations);

        TL::Span<const TL::String> GetSceneGraphs() const;
        TL::Span<const TL::String> GetMeshes() const;
        TL::Span<const TL::String> GetImages() const;
        TL::Span<const TL::String> GetMaterials() const;

        template<typename Archive>
        void Serialize(Archive& archive) const
        {
            TL::Encode(archive, m_exportName);
            TL::Encode(archive, m_uniqueID);
            TL::Encode(archive, m_sceneGraphs);
            TL::Encode(archive, m_meshes);
            TL::Encode(archive, m_images);
            TL::Encode(archive, m_materials);
        }

        template<typename Archive>
        void Deserialize(Archive& archive)
        {
            TL::Decode(archive, m_exportName);
            TL::Decode(archive, m_uniqueID);
            TL::Decode(archive, m_sceneGraphs);
            TL::Decode(archive, m_meshes);
            TL::Decode(archive, m_images);
            TL::Decode(archive, m_materials);
        }

    private:
        TL::String m_exportName;
        uint64_t m_uniqueID;
        TL::Vector<TL::String> m_sceneGraphs;
        TL::Vector<TL::String> m_meshes;
        TL::Vector<TL::String> m_images;
        TL::Vector<TL::String> m_materials;
    };
} // namespace Examples::Assets