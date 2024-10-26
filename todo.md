TL:
    - Hot Reloading
    - SmallVector
    - Task Scheduler
    - Multithreading Interface
    - Serialization/Deserialization
    - Reflection
    - Command line interface builder

RHI:
    - Iterate on Render Graph Interface design
    - Commands are recorded into passes, so passes should handle sync data
    - Expose synchornization
    - Secondary command lists
    - Fix swapchain resize issues
    - MSAA
    - Drop RAII completelly (Use Create/Destroy) in similar style to Vulkan.hpp
    - Mesh Pipeline
    - Ray Tracing Pipeline
    - Querys
    - Add Semaphore as a type
    - Add Barrier and Split Barriers as CommandList Commands
    - Add BeginRenderPass and EndRenderPass commands
    - Add SubmitInfo struct


RPI:
    - ImGui
    - Scene
    - Material
    - Mesh
    - Model
    - Texture
    - Renderer Interfacee
    - Feature Interface
    - View
    - Lights
    - Pipeline Hot Reloading
    - Streaming Resources
    - Shader Reflection Interface (through Slang API)
    - Text Renderer
    - Primitives Renderer

GPU Data:
    Dynamic Uniform Buffer: Lights, Materials, ...etc
        - Lights
        - Materials
        - Transforms
    Instance Buffer: per instasnce transofmrs
    Geometry Buffer: Actual mesh geometry buffers

Asset:
    - Texture/Image Asset
    - Shader Asset
    - Mseh Asset

Deferred Renderer:
    - Shadow Pass
    - SSAO
    - PBR Geometry Renderer

Misc:
    - Extract Examples/Engine code to seperate repo
    - Integrate Physics
    - Integrate ESC
    - Build on Linux
    - Build on Android
    - Setup Github
    - CVars

Editor:
    Properties Tags (tags in code that automtaically create GUI)


TODO:
    - Add staging buffer
    - Refactor render graph