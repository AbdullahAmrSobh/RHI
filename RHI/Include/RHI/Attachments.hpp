#pragma once
#include "RHI/Export.hpp"
#include "RHI/Resources.hpp"
#include "RHI/Common/Ptr.h"
#include "RHI/Common/Hash.hpp"
#include "RHI/Access.hpp"

#include <unordered_map>
#include <string>

namespace std
{
    template<>
    class hash<RHI::ImageViewCreateInfo>
    {
    public:
        inline size_t operator()(const RHI::ImageViewCreateInfo& createInfo) const
        {
            return RHI::HashAny(createInfo);
        }
    };

    template<>
    class hash<RHI::BufferViewCreateInfo>
    {
    public:
        inline size_t operator()(const RHI::BufferViewCreateInfo& createInfo) const
        {
            return RHI::HashAny(createInfo);
        }
    };
} // namespace std

namespace RHI
{
    class Attachment;
    class ImageAttachment;
    class BufferAttachment;
    class PassAttachment;
    class ImagePassAttachment;
    class BufferPassAttachment;
    class Swapchain;
    class Pass;

    enum class LoadOperation
    {
        DontCare, // The attachment load operation undefined.
        Load,     // Load attachment content.
        Discard,  // Discard attachment content.
    };

    enum class StoreOperation
    {
        DontCare, // Attachment Store operation is undefined
        Store,    // Writes to the attachment are stored
        Discard,  // Writes to the attachment are discarded
    };

    struct ColorValue
    {
        float       r = 1.0f;
        float       g = 1.0f;
        float       b = 1.0f;
        float       a = 1.0f;

        inline bool operator==(const ColorValue& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        inline bool operator!=(const ColorValue& other) const
        {
            return !(*this == other);
        }
    };

    struct DepthStencilValue
    {
        float       depthValue   = 1.0f;
        uint8_t     stencilValue = 0xff;

        inline bool operator==(const DepthStencilValue& other) const
        {
            return depthValue == other.depthValue && stencilValue == other.stencilValue;
        }

        inline bool operator!=(const DepthStencilValue& other) const
        {
            return !(*this == other);
        }
    };

    union ClearValue
    {
        ClearValue()
        {
            colorValue = {};
        }

        ClearValue(ColorValue value)
        {
            colorValue = value;
        }

        ClearValue(DepthStencilValue value)
        {
            depthStencilValue = value;
        }

        ColorValue        colorValue;
        DepthStencilValue depthStencilValue;
    };

    struct LoadStoreOperations
    {
        LoadOperation  loadOperation  = LoadOperation::Discard;
        StoreOperation storeOperation = StoreOperation::Store;

        inline bool    operator==(const LoadStoreOperations& other) const
        {
            return loadOperation == other.loadOperation && storeOperation == other.storeOperation;
        }

        inline bool operator!=(const LoadStoreOperations& other) const
        {
            return !(*this == other);
        }
    };

    class RHI_EXPORT Attachment
    {
    public:
        virtual ~Attachment() = default;

        enum class Type
        {
            Image,
            Buffer,
        };

        enum class Lifetime
        {
            Persistent,
            Transient,
        };

        const char*                  m_name;
        const Lifetime               m_lifetime;
        const Type                   m_type;

        inline const PassAttachment* GetFirstPassAttachment() const { return m_firstPassAttachment; }

        inline PassAttachment*       GetFirstPassAttachment() { return m_firstPassAttachment; }

        inline const PassAttachment* GetLastPassAttachment() const { return m_lastPassAttachment; }

        inline PassAttachment*       GetLastPassAttachment() { return m_lastPassAttachment; }

        void                         Insert(PassAttachment* passAttachment);

        void                         Remove(PassAttachment* passAttachment);

        template<typename T>
            requires std::is_base_of_v<Attachment, T>
        T* As()
        {
            if constexpr (std::is_same_v<T, ImageAttachment>)
            {
                if (m_type == Type::Image)
                    return (T*)this;
            }
            if constexpr (std::is_same_v<T, BufferAttachment>)
            {
                if (m_type == Type::Buffer)
                    return (T*)this;
            }
            return nullptr;
        }

    protected:
        Attachment()                  = delete;
        Attachment(const Attachment&) = delete;
        Attachment(Attachment&&)      = delete;

        Attachment(const char* name, Lifetime lifetime, Type type)
            : m_name(name)
            , m_lifetime(lifetime)
            , m_type(type)
            , m_firstPassAttachment(nullptr)
            , m_lastPassAttachment(nullptr)
            , m_referenceCount(0)
        {
            m_asBuffer = {};
            m_asImage  = {};
        }

        PassAttachment* m_firstPassAttachment;
        PassAttachment* m_lastPassAttachment;

        uint32_t        m_referenceCount;

        union
        {
            struct
            {
                Handle<Image>   handle;
                ImageCreateInfo info;
            } m_asImage;

            struct
            {
                Handle<Buffer>   handle;
                BufferCreateInfo info;
            } m_asBuffer;
        };
    };

    class RHI_EXPORT ImageAttachment final : public Attachment
    {
    public:
        inline ImageAttachment(const char* name, Handle<Image> handle)
            : Attachment(name, Lifetime::Persistent, Type::Image)
        {
            m_asImage.handle = handle;
        }

        inline ImageAttachment(const char* name, const ImageCreateInfo& createInfo)
            : Attachment(name, Lifetime::Transient, Type::Image)
        {
            m_asImage.info = createInfo;
        }

        Swapchain*  m_swapchain;

        inline void SetSize(ImageSize2D size)
        {
            m_asImage.info.size.width  = size.width;
            m_asImage.info.size.height = size.height;
            m_asImage.info.size.depth  = 1;
        }

        inline const ImageCreateInfo&     GetCreateInfo() const { return m_asImage.info; }

        inline Handle<Image>              GetHandle() { return m_asImage.handle; }

        inline void                       SetHandle(Handle<Image> handle) { m_asImage.handle = handle; }

        inline const ImagePassAttachment* GetFirstPassAttachment() const { return (const ImagePassAttachment*)m_firstPassAttachment; }

        inline ImagePassAttachment*       GetFirstPassAttachment() { return (ImagePassAttachment*)m_firstPassAttachment; }

        inline const ImagePassAttachment* GetLastPassAttachment() const { return (const ImagePassAttachment*)m_lastPassAttachment; }

        inline ImagePassAttachment*       GetLastPassAttachment() { return (ImagePassAttachment*)m_lastPassAttachment; }

        inline void                       Insert(ImagePassAttachment* passAttachment) { Attachment::Insert((PassAttachment*)passAttachment); }

        inline void                       Remove(ImagePassAttachment* passAttachment) { Attachment::Remove((PassAttachment*)passAttachment); }
    };

    class RHI_EXPORT BufferAttachment final : public Attachment
    {
    public:
        inline BufferAttachment(const char* name, Handle<Buffer> handle)
            : Attachment(name, Lifetime::Persistent, Type::Buffer)
        {
            m_asBuffer.handle = handle;
        }

        inline BufferAttachment(const char* name, size_t size)
            : Attachment(name, Lifetime::Transient, Type::Buffer)
        {
            m_asBuffer.info.byteSize = size;
        }

        inline const BufferCreateInfo&     GetCreateInfo() const { return m_asBuffer.info; }

        inline Handle<Buffer>              GetHandle() { return m_asBuffer.handle; }

        inline void                        SetHandle(Handle<Buffer> handle) { m_asBuffer.handle = handle; }

        inline const BufferPassAttachment* GetFirstPassAttachment() const { return (const BufferPassAttachment*)m_firstPassAttachment; }

        inline BufferPassAttachment*       GetFirstPassAttachment() { return (BufferPassAttachment*)m_firstPassAttachment; }

        inline const BufferPassAttachment* GetLastPassAttachment() const { return (const BufferPassAttachment*)m_lastPassAttachment; }

        inline BufferPassAttachment*       GetLastPassAttachment() { return (BufferPassAttachment*)m_lastPassAttachment; }

        inline void                        Insert(BufferPassAttachment* passAttachment) { Attachment::Insert((PassAttachment*)passAttachment); }

        inline void                        Remove(BufferPassAttachment* passAttachment) { Attachment::Remove((PassAttachment*)passAttachment); }
    };

    class RHI_EXPORT PassAttachment
    {
    public:
        PassAttachment(Attachment* attachment, Pass* pass)
            : m_pass(pass)
            , m_attachment(attachment)
        {
        }

        inline const Attachment*     GetAttachment() const { return m_attachment; }

        inline Attachment*           GetAttachment() { return m_attachment; }

        inline const PassAttachment* GetNext() const { return m_next; }

        inline PassAttachment*       GetNext() { return m_next; }

        inline const PassAttachment* GetPrev() const { return m_prev; }

        inline PassAttachment*       GetPrev() { return m_prev; }

        Pass*                        m_pass;
        Access                       m_access;
        RHI::Flags<RHI::ShaderStage> m_stage;

    protected:
        Attachment* m_attachment;

    private:
        friend class Attachment;
        PassAttachment* m_next;
        PassAttachment* m_prev;
    };

    class RHI_EXPORT ImagePassAttachment final : public PassAttachment
    {
    public:
        inline ImagePassAttachment(ImageAttachment* attachment, Pass* pass)
            : PassAttachment((Attachment*)attachment, pass)
        {
        }

        inline const ImageAttachment*     GetAttachment() const { return (const ImageAttachment*)m_attachment; }

        inline ImageAttachment*           GetAttachment() { return (ImageAttachment*)m_attachment; }

        inline const ImagePassAttachment* GetNext() const { return (ImagePassAttachment*)PassAttachment::GetNext(); }

        inline ImagePassAttachment*       GetNext() { return (ImagePassAttachment*)PassAttachment::GetNext(); }

        inline const ImagePassAttachment* GetPrev() const { return (ImagePassAttachment*)PassAttachment::GetPrev(); }

        inline ImagePassAttachment*       GetPrev() { return (ImagePassAttachment*)PassAttachment::GetPrev(); }

        ClearValue                        m_clearValue;
        LoadStoreOperations               m_loadStoreOperations;
        ImageViewCreateInfo               m_viewInfo;
        Handle<ImageView>                 m_view;
        ImageUsage                        m_usage;
    };

    class RHI_EXPORT BufferPassAttachment final : public PassAttachment
    {
    public:
        inline BufferPassAttachment(BufferAttachment* attachment, Pass* pass)
            : PassAttachment((Attachment*)attachment, pass)
        {
        }

        inline const BufferAttachment*     GetAttachment() const { return (const BufferAttachment*)m_attachment; }

        inline BufferAttachment*           GetAttachment() { return (BufferAttachment*)m_attachment; }

        inline const BufferPassAttachment* GetNext() const { return (BufferPassAttachment*)PassAttachment::GetNext(); }

        inline BufferPassAttachment*       GetNext() { return (BufferPassAttachment*)PassAttachment::GetNext(); }

        inline const BufferPassAttachment* GetPrev() const { return (BufferPassAttachment*)PassAttachment::GetPrev(); }

        inline BufferPassAttachment*       GetPrev() { return (BufferPassAttachment*)PassAttachment::GetPrev(); }

        BufferUsage                        m_usage;
        BufferViewCreateInfo               m_viewInfo;
        Handle<BufferView>                 m_view;
    };

    class AttachmentsPool final
    {
    public:
        AttachmentsPool(Context* context);

        void                  InitPassAttachment(PassAttachment* passAttachment);
        void                  ShutdownPassAttachment(PassAttachment* passAttachment);

        ImageAttachment*      NewImageAttachment(const char* name, Handle<Image> handle);
        ImageAttachment*      NewImageAttachment(const char* name, Format format, ImageType type, ImageSize3D size, SampleCount sampleCount, uint32_t mipLevelsCount, uint32_t arrayLayersCount);
        BufferAttachment*     NewBufferAttachment(const char* name, size_t size);
        void                  DestroyAttachment(Attachment* attachment);

        Handle<ImageView>     CreateImageView(const ImageViewCreateInfo& createInfo);
        Handle<BufferView>    CreateBufferView(const BufferViewCreateInfo& createInfo);

        TL::Span<Attachment*> GetAttachments() { return m_attachments; }

        TL::Span<Attachment*> GetTransientAttachments() { return m_transientAttachments; }

    private:
        Context*                                                     m_context;

        std::unordered_map<std::string, Ptr<Attachment>>             m_attachmentsLut;
        std::vector<Attachment*>                                     m_attachments;
        std::vector<Attachment*>                                     m_transientAttachments;
        std::vector<ImageAttachment*>                                m_imageAttachments;
        std::vector<BufferAttachment*>                               m_bufferAttachments;

        std::unordered_map<ImageViewCreateInfo, Handle<ImageView>>   m_imageViewsLRU;
        std::unordered_map<BufferViewCreateInfo, Handle<BufferView>> m_bufferViewsLRU;
    };
} // namespace RHI