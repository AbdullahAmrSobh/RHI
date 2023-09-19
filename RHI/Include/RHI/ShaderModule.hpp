#pragma once

#include "RHI/Common.hpp"

namespace RHI
{

enum class ShaderStage
{
    Vertex,
    Pixel,
    Compute,
};

struct ShaderModuleCreateInfo
{
    Flags<ShaderStage> stages;

    const char* vertexName;
    const char* pixelName;
    const char* computeName;

    void*  code;
    size_t size;
};

class ShaderModule : public Object
{
public:
    using Object::Object;
    virtual ~ShaderModule() = default;
};

}  // namespace RHI