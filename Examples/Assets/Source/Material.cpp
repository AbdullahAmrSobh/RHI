#include "Assets/Material.hpp"

namespace Examples::Assets
{
    // template<typename T>
    // void Material::AddProperty(const char* name, T value)
    // {
    //     m_properties.insert_or_assign(name, MaterialProperty(value));
    // }

    void Material::AddTexture(const char* name, Name textureName)
    {
        m_textures.insert_or_assign(name, textureName);
    }

    // template<typename T>
    // T Material::GetProperty(const char* name) const
    // {
    //     auto it = m_properties.find(name);
    //     if (it != m_properties.end())
    //     {
    //         return std::get<T>(it->second.value); // Access the value using std::get<T>
    //     }
    //     else
    //     {
    //         TL_LOG_ERROR("Property not found: ", name);
    //         return T{}; // Return a default-constructed T if not found
    //     }
    // }

    Name Material::GetTexture(const char* name) const
    {
        auto it = m_textures.find(name);
        if (it != m_textures.end())
        {
            return it->second;
        }
        else
        {
            TL_LOG_ERROR("Texture not found: ", name);
            return Name(); // Return a default-constructed Name if not found
        }
    }

} // namespace Examples::Assets
