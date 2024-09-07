#pragma once

#include "Assets/Export.hpp"
#include "Assets/Package.hpp"
#include "Assets/Image.hpp"
#include "Assets/Mesh.hpp"
#include "Assets/SceneGraph.hpp"
#include "Assets/Material.hpp"

namespace Examples::Assets
{
    struct ImportInfo
    {
        const char* filePath;
        const char* outputPath;
        const char* packageName;
    };

    /// @brief imports a given scene and generates a Assets Package
    ASSETS_EXPORT Package Import(const ImportInfo& importInfo);
} // namespace Examples::Assets