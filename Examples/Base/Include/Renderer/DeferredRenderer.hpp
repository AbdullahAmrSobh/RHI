#pragma once

#include "Examples-Base/Renderer.hpp"
#include <RHI/RenderGraph.hpp>

namespace Examples
{
    Renderer* CreateDeferredRenderer();

    class DeferredRenderer;
}