#pragma once
#include "RHI/Common.hpp"
#include "RHI/FrameGraphAttachmentsRegistry.hpp"

namespace RHI
{
    
enum class EPassType;
enum class EAttachmentUsage;
enum class EAttachmentAccess;

struct ImagePassAttachmentDesc;
struct BufferPassAttachmentDesc;

class IDevice;
class ISwapchain;
class IPass;
class IPassProducer;

class IFrameGraph
{
public:
    virtual ~IFrameGraph() = default;

    IAttachmentsRegistry& GetAttachmentsRegistry();
    const IAttachmentsRegistry& GetAttachmentsRegistry() const;

    EResultCode BeginFrame();
    EResultCode EndFrame();

    EResultCode ImportPassProducer(IPassProducer& producer, std::string name, EPassType type); 
    
    virtual Expected<Unique<IPass>> CreatePass(std::string name, EPassType type) = 0;

    virtual EResultCode Execute(const IPassProducer& producer) = 0;

private:
    Unique<IAttachmentsRegistry> m_attachmentsRegistry; 
    
    std::vector<IPassProducer*> m_producers; 
};

} // namespace RHI
