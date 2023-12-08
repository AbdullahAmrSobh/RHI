#pragma once

#include "RHI/Common/Flags.hpp"
#include "RHI/Common/Handle.hpp"
#include "RHI/Common/Result.hpp"
#include "RHI/Common/Span.hpp"
#include "RHI/Format.hpp"

#include <unordered_map>
#include <variant>

namespace RHI
{
    class Context;
    class ShaderModule;
    class Pass;

    // clang-format off
    struct Image {};
    struct Buffer {};
    struct ImageView {};
    struct BufferView {};
    struct BindGroupLayout {};
    struct BindGroup {};
    struct PipelineLayout {};
    struct GraphicsPipeline {};
    struct ComputePipeline {};
    struct Sampler {};
    // clang-format on

    struct ImagePassAttachment;
    struct BufferPassAttachment;

    /// @brief Represents a pointer to GPU device memory
    using DeviceMemoryPtr = void*;

    /// @brief Enumeration for common allocation size constants
    namespace AllocationSizeConstants
    {
        inline static constexpr uint32_t KB = 1024;
        inline static constexpr uint32_t MB = 1024 * KB;
        inline static constexpr uint32_t GB = 1024 * MB;

    }; // namespace AllocationSizeConstants

    /// @brief Enumeration representing the memory allocation startegy of a resource pool.
    enum class AllocationAlgorithm
    {
        /// @brief Memory will be allocated linearly.
        Linear,
    };

    /// @brief Enumeration representing different memory types and their locations.
    enum class MemoryType
    {
        CPU,       // The memory is located in the system main memory.
        GPULocal,  // The memory is locaed in the GPU, and can't be accessed by the CPU.
        GPUShared, // The memory is locaed in the GPU, but can be accessed by the CPU.
    };

    /// @brief Enumerates the multisample count in an image or an graphics pipeline multisample state
    enum class SampleCount
    {
        None      = 0 << 0,
        Samples1  = 1 << 0,
        Samples2  = 1 << 1,
        Samples4  = 1 << 2,
        Samples8  = 1 << 3,
        Samples16 = 1 << 4,
        Samples32 = 1 << 5,
        Samples64 = 1 << 6,
    };

    /// @brief Enumeration representing how the image resource is intented to used.
    enum class ImageUsage
    {
        None           = 0 << 0, // Invalid flag
        ShaderResource = 1 << 1, // The image will be used in an shader as bind resource.
        Color          = 1 << 3, // The image will be the render target color attachment.
        Depth          = 1 << 4, // The image will be the render target depth attachment.
        Stencil        = 1 << 5, // The image will be the render target stencil attachment.
        CopySrc        = 1 << 6, // The image content will be copied.
        CopyDst        = 1 << 7, // The image content will be overwritten by a copy command.
    };

    /// @brief Enumeration representing the dimensions of an image resource.
    enum class ImageType
    {
        None,    // Invalid flag
        Image1D, // Image is 1 dimentional.
        Image2D, // Image is 2 dimentional.
        Image3D, // Image is 3 dimentional.
    };

    /// @brief Enumeration representing the aspects of an image resource.
    enum class ImageAspect
    {
        None         = 0,
        Color        = 1 << 1,
        Depth        = 1 << 2,
        Stencil      = 1 << 3,
        DepthStencil = Depth | Stencil,
        All          = Color | DepthStencil,
    };

    /// @brief Enumeration representing the component mapping.
    enum class ComponentSwizzle
    {
        Identity = 0,
        Zero,
        One,
        R,
        G,
        B,
        A,
    };

    /// @brief Enumeration representing how the buffer resource is intented to used.
    enum class BufferUsage
    {
        None    = 0 << 0,
        Storage = 1 << 1, // The buffer will be used as a storage buffer object.
        Uniform = 1 << 2, // The buffer will be used as an uniform buffer object.
        Vertex  = 1 << 3, // The buffer will be used as a vertex buffer object.
        Index   = 1 << 4, // The buffer will be used as a index buffer object.
        CopySrc = 1 << 5, // This buffer content will be copied from.
        CopyDst = 1 << 6, // This buffer content will be overwritten by a copy command.
    };

    enum class ShaderStage
    {
        None    = 0 << 0,
        Vertex  = 1 << 1,
        Pixel   = 1 << 2,
        Compute = 1 << 3,
    };

    /// @brief The type of the shader resource to be bound.
    enum class ShaderBindingType
    {
        None,
        Sampler,
        Image,
        Buffer,
    };

    /// @brief How the resource will be accessed in the shader.
    enum class ShaderBindingAccess
    {
        OnlyRead,
        ReadWrite,
    };

    /// @brief Pipeline vertex
    enum class PipelineVertexInputRate
    {
        PerInstance,
        PerVertex,
    };

    /// @brief Pipeline rasterizer state cull mode.
    enum class PipelineRasterizerStateCullMode
    {
        None,
        FrontFace,
        BackFace,
        Discard,
    };

    /// @brief Pipeline rasterizer state fill mode.
    enum class PipelineRasterizerStateFillMode
    {
        Point,
        Triangle,
        Line
    };

    /// @brief Pipeline topology mode.
    enum class PipelineTopologyMode
    {
        Points,
        Lines,
        Triangles,
    };

    /// @brief The orientation of triangle fornt faces
    enum class PipelineRasterizerStateFrontFace
    {
        Clockwise,
        CounterClockwise,
    };

    /// @brief Operator used to compare two values
    enum class CompareOperator
    {
        Never,
        Equal,
        NotEqual,
        Greater,
        GreaterOrEqual,
        Less,
        LessOrEqual,
        Always,
    };

    /// @brief Enumeration representing the sampler filtering.
    enum class SamplerFilter
    {
        Point,
        Linear,
    };

    /// @brief Enumeration representing the sampler addressing mode.
    enum class SamplerAddressMode
    {
        Repeat,
        Clamp,
    };

    /// @brief Enumeration representing the sampler compare opertion.
    enum class SamplerCompareOperation
    {
        Never,
        Equal,
        NotEqual,
        Always,
        Less,
        LessEq,
        Greater,
        GreaterEq,
    };

    /// @brief Enumerates ...
    enum class BlendFactor
    {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
    };

    /// @brief Enumerates ...
    enum class BlendEquation
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    enum class ColorWriteMask
    {
        Red   = 0x01,
        Green = 0x02,
        Blue  = 0x04,
        Alpha = 0x08,
        All   = Red | Green | Blue | Alpha,
    };

    enum class MemoryAllocationFlags
    {
        None,
        LazilyAllocated,
        Mappable
    };

#ifdef RHI_PLATFORM_WINDOWS
    /// @brief struct contains win32 surface handles.
    struct Win32WindowDesc
    {
        void* hwnd;
        void* hinstance;
    };
#endif

    /// @brief Report describe the current state of the resource pool.
    struct PoolReport
    {
        MemoryType type;
        float      fragmentation;
        size_t     size;
        size_t     alignment;
        size_t     allocationsCount;
    };

    /// @brief Represent the offset into an image resource.
    struct ImageOffset
    {
        int32_t     x; // Offset in the X direction
        int32_t     y; // Offset in the Y direction
        int32_t     z; // Offset in the Z direction

        inline bool operator==(const ImageOffset& other) const
        {
            return x == other.x && y == other.y && z == other.z;
        }

        inline bool operator!=(const ImageOffset& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent the size of an image resource or subregion.
    struct ImageSize2D
    {
        uint32_t    width;  // The width of the image.
        uint32_t    height; // The height of the image.

        inline bool operator==(const ImageSize2D& other) const
        {
            return width == other.width && height == other.height;
        }

        inline bool operator!=(const ImageSize2D& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent the size of an image resource or subregion.
    struct ImageSize
    {
        uint32_t    width;  // The width of the image.
        uint32_t    height; // The height of the image.
        uint32_t    depth;  // The depth of the image.

        inline bool operator==(const ImageSize& other) const
        {
            return width == other.width && height == other.height && depth == other.depth;
        }

        inline bool operator!=(const ImageSize& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent the texel color swizzling operation
    struct ComponentMapping
    {
        ComponentMapping() = default;

        ComponentSwizzle r = ComponentSwizzle::R;
        ComponentSwizzle g = ComponentSwizzle::G;
        ComponentSwizzle b = ComponentSwizzle::B;
        ComponentSwizzle a = ComponentSwizzle::A;

        inline bool      operator==(const ComponentMapping& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        inline bool operator!=(const ComponentMapping& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent a subview into a an image resource.
    struct ImageSubresource
    {
        ImageSubresource()              = default;

        uint32_t           arrayBase    = 0;
        uint32_t           arrayCount   = 1;
        uint32_t           mipBase      = 0;
        uint32_t           mipCount     = 1;
        Flags<ImageAspect> imageAspects = ImageAspect::All;

        inline bool        operator==(const ImageSubresource& other) const
        {
            return arrayBase == other.arrayBase && arrayCount == other.arrayCount && mipBase == other.mipBase && mipCount == other.mipCount;
        }

        inline bool operator!=(const ImageSubresource& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent a subview into a an buffer resource.
    struct BufferSubregion
    {
        BufferSubregion() = default;

        size_t      byteSize;
        size_t      byteOffset;
        Format      format;

        inline bool operator==(const BufferSubregion& other) const
        {
            return byteSize == other.byteSize && byteOffset == other.byteOffset && format == other.format;
        }

        inline bool operator!=(const BufferSubregion& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Specifies a single shader resource binding.
    struct ShaderBinding
    {
        ShaderBindingType       type;
        ShaderBindingAccess     access;
        uint32_t                arrayCount;
        Flags<RHI::ShaderStage> stages;
    };

    /// @brief A shader bind group layout is an list of shader bindings.
    struct BindGroupLayoutCreateInfo
    {
        BindGroupLayoutCreateInfo(std::initializer_list<ShaderBinding> initList)
            : bindings(initList)
        {
        }

        std::vector<ShaderBinding> bindings;
    };

    // @brief the layout of pipeline shaders
    struct PipelineLayoutCreateInfo
    {
        PipelineLayoutCreateInfo(TL::Span<Handle<BindGroupLayout>> bindGroupLayouts)
            : layouts{ bindGroupLayouts.begin(), bindGroupLayouts.end() }
        {
        }

        std::vector<Handle<BindGroupLayout>> layouts;
    };

    /// @brief Structure specifying the blending parameters for an image render target attachment.
    struct ColorAttachmentBlendStateDesc
    {
        bool                  blendEnable  = false;
        BlendEquation         colorBlendOp = BlendEquation::Add;
        BlendFactor           srcColor     = BlendFactor::One;
        BlendFactor           dstColor     = BlendFactor::Zero;
        BlendEquation         alphaBlendOp = BlendEquation::Add;
        BlendFactor           srcAlpha     = BlendFactor::One;
        BlendFactor           dstAlpha     = BlendFactor::Zero;
        Flags<ColorWriteMask> writeMask    = ColorWriteMask::All;

        inline bool           operator==(const ColorAttachmentBlendStateDesc& other) const
        {
            return blendEnable == other.blendEnable && colorBlendOp == other.colorBlendOp && srcColor == other.srcColor && dstColor == other.dstColor && alphaBlendOp == other.alphaBlendOp && srcAlpha == other.srcAlpha && dstAlpha == other.dstAlpha;
        }

        inline bool operator!=(const ColorAttachmentBlendStateDesc& other) const
        {
            return !(*this == other);
        }
    };

    struct ShaderStageDesc
    {
    };

    /// @brief Structure specifying the render target layout.
    struct PipelineRenderTargetLayout
    {
        TL::Span<const Format> colorAttachmentsFormats; // List of the formats of color attachments.
        Format                 depthAttachmentFormat;   // Format of an optional depth and/or stencil attachment.
        Format                 stencilAttachmentFormat; // Format of an optional depth and/or stencil attachment.
    };

    struct PipelineVertexBindingDesc
    {
        uint32_t                binding;
        uint32_t                stride;
        PipelineVertexInputRate stepRate;
    };

    struct PipelineVertexAttributeDesc
    {
        uint32_t location;
        uint32_t binding;
        Format   format;
        uint32_t offset;
    };

    struct PipelineInputAssemblerStateDesc
    {
        std::vector<PipelineVertexBindingDesc>   bindings;
        std::vector<PipelineVertexAttributeDesc> attributes;
    };

    /// @brief Structure specifying the rasterizer state.
    struct PipelineRasterizerStateDesc
    {
        PipelineRasterizerStateCullMode  cullMode  = PipelineRasterizerStateCullMode::BackFace;
        PipelineRasterizerStateFillMode  fillMode  = PipelineRasterizerStateFillMode::Triangle;
        PipelineRasterizerStateFrontFace frontFace = PipelineRasterizerStateFrontFace::CounterClockwise;
        float                            lineWidth = 1.0f;
    };

    /// @brief Structure specifying the multisample state.
    struct PipelineMultisampleStateDesc
    {
        SampleCount sampleCount;
        bool        sampleShading;
    };

    /// @brief Structure specifying the depth and stencil state.
    struct PipelineDepthStencilStateDesc
    {
        bool            depthTestEnable;
        bool            depthWriteEnable;
        CompareOperator compareOperator;
        bool            stencilTestEnable;
    };

    /// @brief Structure specifying the color attachments blend state.
    struct PipelineColorBlendStateDesc
    {
        TL::Span<const ColorAttachmentBlendStateDesc> blendStates;
        float                                         blendConstants[4];
    };

    /// @brief Represent the creation parameters of an resource pool.
    struct PoolCreateInfo
    {
        AllocationAlgorithm allocationAlgorithm = AllocationAlgorithm::Linear;
        MemoryType          heapType;
        size_t              blockSize;
        size_t              minBlockCount;
        size_t              maxBlockCount;
        size_t              minBlockAlignment;
    };

    /// @brief Represent the creation parameters of an image resource.
    struct ImageCreateInfo
    {
        Flags<ImageUsage> usageFlags;                               // Usage flags.
        ImageType         type;                                     // The type of the image.
        ImageSize         size;                                     // The size of the image.
        Format            format;                                   // The format of the image.
        SampleCount       sampleCount = RHI::SampleCount::Samples1; // The number of samples in each texel.
        uint32_t          mipLevels   = 1;                          // The number of mip levels in the image.
        uint32_t          arrayCount  = 1;                          // The number of images in the images array.

        inline bool       operator==(const ImageCreateInfo& other) const
        {
            return usageFlags == other.usageFlags && type == other.type && size == other.size && format == other.format && mipLevels == other.mipLevels && arrayCount == other.arrayCount;
        }

        inline bool operator!=(const ImageCreateInfo& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represent the creation parameters of an buffer resource.
    struct BufferCreateInfo
    {
        Flags<BufferUsage> usageFlags; // Usage flags.
        size_t             byteSize;   // The size of the buffer.

        inline bool        operator==(const BufferCreateInfo& other) const
        {
            return usageFlags == other.usageFlags && byteSize == other.byteSize;
        }

        inline bool operator!=(const BufferCreateInfo& other) const
        {
            return !(*this == other);
        }
    };

    struct ShaderModuleCreateInfo
    {
        void*  code;
        size_t size;
    };

    /// @brief Description of the graphics pipeline states.
    struct GraphicsPipelineCreateInfo
    {
        const char*                     vertexShaderName;
        ShaderModule*                   vertexShaderModule;

        const char*                     pixelShaderName;
        ShaderModule*                   pixelShaderModule;

        Handle<PipelineLayout>          layout;

        PipelineInputAssemblerStateDesc inputAssemblerState;
        PipelineTopologyMode            topologyMode;
        PipelineRenderTargetLayout      renderTargetLayout;
        PipelineRasterizerStateDesc     rasterizationState;
        PipelineMultisampleStateDesc    multisampleState;
        PipelineDepthStencilStateDesc   depthStencilState;
        PipelineColorBlendStateDesc     colorBlendState;
    };

    /// @brief Description of a compute pipeline state.
    struct ComputePipelineCreateInfo
    {
        const char*            shaderName;
        ShaderModule*          shaderModule;
        Handle<PipelineLayout> layout;
    };

    /// @brief Structure describing the creation parameters of a sampler state.
    struct SamplerCreateInfo
    {
        SamplerCreateInfo()                = default;

        SamplerFilter           filterMin  = SamplerFilter::Point;
        SamplerFilter           filterMag  = SamplerFilter::Point;
        SamplerFilter           filterMip  = SamplerFilter::Point;
        SamplerCompareOperation compare    = SamplerCompareOperation::Always;
        float                   mipLodBias = 0.0f;
        SamplerAddressMode      addressU   = SamplerAddressMode::Clamp;
        SamplerAddressMode      addressV   = SamplerAddressMode::Clamp;
        SamplerAddressMode      addressW   = SamplerAddressMode::Clamp;
        float                   minLod     = 0.0f;
        float                   maxLod     = 1.0f;

        inline bool             operator==(const SamplerCreateInfo& other) const
        {
            return filterMin == other.filterMin && filterMag == other.filterMag && filterMip == other.filterMip && compare == other.compare && mipLodBias == other.mipLodBias && addressU == other.addressU && addressV == other.addressV && addressW == other.addressW && minLod == other.minLod && maxLod == other.maxLod;
        }

        inline bool operator!=(const SamplerCreateInfo& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Structure specifying the parameters of the swapchain.
    struct SwapchainCreateInfo
    {
        ImageSize         imageSize;   // The size of the images in the swapchian.
        Flags<ImageUsage> imageUsage;  // Image usage flags applied to all created images.
        Format            imageFormat; // The format of created swapchain image.
        uint32_t          imageCount;  // The numer of back buffer images in the swapchain.
#ifdef RHI_PLATFORM_WINDOWS
        Win32WindowDesc win32Window; // win32 surface handles. (Availabe only on windows)
#endif
    };

    // struct PipelineFactoryCreateInfo
    // {
    //     void*  cacheData;
    //     size_t dataSize;
    // };

    // class PipelineCache {};

    // class PipelineFactory
    // {
    // public:
    //     ~PipelineFactory() = default;
    //     virtual PipelineCache GetCache() const = 0;
    //     virtual std::unique_ptr<GraphicsPipeline> CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;
    //     virtual std::unique_ptr<ComputePipeline> CreateComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;
    //     virtual std::unique_ptr<RayTracingPipeline> CreateRayTracingPipeline(const RayTracingPipelineCreateInfo& createInfo) = 0;
    // };

    /// @brief An object that groups shader resources that are bound together.
    class RHI_EXPORT BindGroupData final
    {
    public:
        BindGroupData() = default;

        /// @brief Binds an image resource to the provided binding index and offset array index.
        /// NOTE: offset + images count should not exceed the count of the resources decalred in the layout or the shader.
        /// @param index index of the resource binding decelration in the shader.
        /// @param images list of handles of an actual resources to bind.
        /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
        void BindImages(uint32_t index, TL::Span<ImagePassAttachment*> images, uint32_t arrayOffset = 0);

        /// @brief Binds an image resource to the provided binding index and offset array index.
        /// NOTE: offset + buffers count should not exceed the count of the resources decalred in the layout or the shader.
        /// @param index index of the resource binding decelration in the shader.
        /// @param buffers list of handles of an actual resources to bind.
        /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
        void BindBuffers(uint32_t index, TL::Span<BufferPassAttachment*> buffers, uint32_t arrayOffset = 0);

        /// @brief Binds an image resource to the provided binding index and offset array index.
        /// NOTE: offset + samplers count should not exceed the count of the resources decalred in the layout or the shader.
        /// @param index index of the resource binding decelration in the shader.
        /// @param samplers list of handles of an actual resources to bind.
        /// @param arrayOffset starting offset. In case of an resources array it binds the resources starting at this number.
        void BindSamplers(uint32_t index, TL::Span<Handle<Sampler>> samplers, uint32_t arrayOffset = 0);

        struct ResourceImageBinding
        {
            uint32_t                          arrayOffset;

            std::vector<ImagePassAttachment*> views;
        };

        struct ResourceBufferBinding
        {
            uint32_t                           arrayOffset;

            std::vector<BufferPassAttachment*> views;
        };

        struct ResourceSamplerBinding
        {
            uint32_t                     arrayOffset;

            std::vector<Handle<Sampler>> samplers;
        };

        using ResourceBinding = std::variant<ResourceImageBinding, ResourceBufferBinding, ResourceSamplerBinding>;

        std::unordered_map<uint32_t, ResourceBinding> m_bindings;
    };

    class RHI_EXPORT BindGroupAllocator
    {
    public:
        BindGroupAllocator()                                                                                          = default;
        virtual ~BindGroupAllocator()                                                                                 = default;

        virtual std::vector<Handle<BindGroup>> AllocateBindGroups(TL::Span<Handle<BindGroupLayout>> bindGroupLayouts) = 0;

        virtual void                           Free(TL::Span<Handle<BindGroup>> groups)                               = 0;

        virtual void                           Update(Handle<BindGroup> group, const BindGroupData& data)             = 0;
    };

    class RHI_EXPORT ShaderModule
    {
    public:
        ShaderModule()          = default;
        virtual ~ShaderModule() = default;
    };

    /// @brief Pool used to allocate buffer resources.
    class RHI_EXPORT BufferPool
    {
    public:
        BufferPool()                                                                = default;
        virtual ~BufferPool()                                                       = default;

        /// @brief Allocate a buffer resource.
        virtual Result<Handle<Buffer>> Allocate(const BufferCreateInfo& createInfo) = 0;

        /// @brief Free an allocated buffer resource.
        virtual void                   FreeBuffer(Handle<Buffer> handle)            = 0;

        /// @brief Get the size of an allocated buffer resource.
        virtual size_t                 GetSize(Handle<Buffer> handle) const         = 0;

        /// @brief Maps the buffer resource for read or write operations.
        /// @return returns a pointer to GPU memory, or a nullptr in case of failure
        virtual DeviceMemoryPtr        MapBuffer(Handle<Buffer> handle)             = 0;

        /// @brief UnmapBuffers the buffer resource.
        virtual void                   UnmapBuffer(Handle<Buffer> handle)           = 0;
    };

    /// @brief Pool used to allocate image resources.
    class RHI_EXPORT ImagePool
    {
    public:
        /// @brief Allocate an image resource.
        virtual Result<Handle<Image>> Allocate(const ImageCreateInfo& createInfo) = 0;

        /// @brief Free an allocated image resource.
        virtual void                  FreeImage(Handle<Image> handle)              = 0;

        /// @brief Get the size of an allocated image resource.
        virtual size_t                GetSize(Handle<Image> handle) const          = 0;
    };

    /// @brief Swapchain object which is an interface between the API and a presentation surface.
    class RHI_EXPORT Swapchain
    {
    public:
        Swapchain()          = default;
        virtual ~Swapchain() = default;

        /// @brief Get the current image index of the swapchain.
        uint32_t           GetCurrentImageIndex() const;

        /// @brief Get the number of images in the swapchain.
        uint32_t           GetImagesCount() const;

        /// @brief Get the current acquired swapchain image.
        Handle<Image>      GetImage() const;

        /// @brief Called to invalidate the current swapchain state, when the window is resized.
        virtual ResultCode Resize(uint32_t newWidth, uint32_t newHeight) = 0;

        /// @brief Presents the current image to the window, and acquires the next image in the swapchain.
        virtual ResultCode Present(Pass& pass)                           = 0;

    protected:
        uint32_t                   m_currentImageIndex;
        uint32_t                   m_swapchainImagesCount;
        std::vector<Handle<Image>> m_images;
    };

    inline void BindGroupData::BindImages(uint32_t index, TL::Span<ImagePassAttachment*> images, uint32_t arrayOffset)
    {
        BindGroupData::ResourceImageBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.views       = { images.begin(), images.end() };
        m_bindings[index]   = binding;
    }

    inline void BindGroupData::BindBuffers(uint32_t index, TL::Span<BufferPassAttachment*> buffers, uint32_t arrayOffset)
    {
        BindGroupData::ResourceBufferBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.views       = { buffers.begin(), buffers.end() };
        m_bindings[index]   = binding;
    }

    inline void BindGroupData::BindSamplers(uint32_t index, TL::Span<Handle<Sampler>> samplers, uint32_t arrayOffset)
    {
        BindGroupData::ResourceSamplerBinding binding{};
        binding.arrayOffset = arrayOffset;
        binding.samplers    = { samplers.begin(), samplers.end() };
        m_bindings[index]   = binding;
    }

    inline uint32_t Swapchain::GetCurrentImageIndex() const
    {
        return m_currentImageIndex;
    }

    inline uint32_t Swapchain::GetImagesCount() const
    {
        return m_swapchainImagesCount;
    }

    inline Handle<Image> Swapchain::GetImage() const
    {
        return m_images[m_currentImageIndex];
    }

} // namespace RHI