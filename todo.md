- Fix validation errors
- Fix memory leaks
- Swapchain and Render Graph Resizing (recompile graph)
- Multipass rendering
- Query Pool
- Tracy Zones
- Tracy Vulkan backend integeration
- Render Graph Roredering and proper sync
- Async Compute
- Async Transfer
- Staging resource
- Mesh Shaders
- Ray Tracing Pipelines
- Buffer Suballocator (Uniforms, Geoemtry, ...etc)
- ImGui
- Fix Camera

- Render Graph Builder Macros

- Cleanup code, Fix runtime and exit leaks.
- Fix swapchain
- Better code quality

Fix deletion queue
Fix swapchain
Imporve RenderGraph API
Ensure there are no memory or resource leaks in the codebase
Cleanup API
    - Remove unused functions, variables, unnecessary includes, Includes, etc.
    - Improve UsageFlags, make unified model

NOTES:
    - ShaderModule is a pointer, maybe Pipeline<T>CreateInfo should take raw pointer to shader byte code instead?
    Too much API noise, Function that create/destroy doesn't match in signatures.
    - The texture to buffer and buffer to texture copy interface should be simplified a bit.
    <!-- - BlitImage is not portable for WebGPU and D3D12 APIs -->
    - ImageSubresourcesX are too many and redundnt probably don't need all of these?
    - Buffer Usage and Image Usage do not map well to D3D12 API (too explicit?)
    <!-- - CompareOperator and SamplerCompare function are same -->
Scene:
    - Mesh
    - Material
    - Texture


WGPU backend
1. Create resources
2. Create Swapchain
3. Render triangle
4. Run the base example
5. No leaks no issues should

D3D

Sort declerations


Idea:
    - create buffer pool as an API first class object (allow suballocations from buffer)
    - Handle type rename to ID or remove the need to wrap structs in Handle ... make typing too long
    - Texel buffers support
    - Don't use singletons (plan to run multiple renderers side-by-side)
