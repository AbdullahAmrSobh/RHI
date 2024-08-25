#pragma once

#include "Assets/Export.hpp"
#include "Assets/Package.hpp"

#include <filesystem>

namespace Examples::Assets
{
    /// @brief imports a given scene and generates a Assets Package
    ASSETS_EXPORT Package Import(std::filesystem::path file);
} // namespace Examples::Assets