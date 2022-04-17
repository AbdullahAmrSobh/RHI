#pragma once
#include "RHI/Definitions.hpp"

namespace RHI
{

enum class ERenderPassQueueType
{
    Compute,
    Graphics,
    Transfer,
};

enum class EAttachmentAccess
{
	Undefined,
    Read,
    Write,
    ReadWrite
};

enum class EAttachmentAccessPipelineStage
{
	Undefined,
	Graphics,
	Compute,
	VertexShader,
	GeometryShader,
	HullShader,
	DomainShader,
	MeshShader,
	PixelShader,
};

enum class EAttachmentUsage
{
	Undefined,
    Color,
    DepthStencil,
    Input,
    Resolve,
    Copy
};

enum class EAttachmentType
{
    Proxy,
    Transient,
};

enum class EAttachmentLoadOperation
{
    Load,
    Clear,
    Undefined
};

enum class EAttachmentStoreOperation
{
    Store,
    Undefined
};

struct AttachmentLoadStoreOperations
{
    EAttachmentLoadOperation  loadOperation;
    EAttachmentStoreOperation storeOperation;
};

} // namespace RHI
