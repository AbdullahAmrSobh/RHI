TL:
    - Hot Reloading
    - SmallVector
    - Task Scheduler
    - Multithreading Interface
    - Serialization/Deserialization
    - Reflection
    - Command line interface builder

RHI:
    - Remove view objects, by either
        - [] adopt similar design to metal with MakeImageView which would create a new image handle
        - [] ResourceViewInfoDesc struct is passed to bind group update function (uses a hash map to generete actual views)
    - unify API semaphore and fence objects into single object (choose name) (timeline semaphores)
        - and rework queue submits accordingly
    - adopt similar API design to WebGPU command encoder
    - DROP RAII
    - Query
    - Explicit Memory Management support
    -

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