#pragma once
#include "RHI/Definitions.hpp"
#include "RHI/Resources.hpp"

namespace RHI
{

class IShaderBytecode
{
public:
	virtual ~IShaderBytecode() = default;
	
};

namespace PipelineStateDesc
{
    struct VertexBufferLayout
    {
        enum class EAttributeType
        {
            U32X1,
            U32X2,
            U32X3,
            U32X4,
            I32X1,
            I32X2,
            I32X3,
            I32X4,
            F32X1,
            F32X2,
            F32X3,
            F32X4,
            F64X1,
            F64X2,
            F64X3,
            F64X4,
        };

        static uint32_t SizeOfAttributeType(EAttributeType type)
        {
            switch (type)
            {
            case EAttributeType::U32X1: return sizeof(uint32_t) * 1;
            case EAttributeType::U32X2: return sizeof(uint32_t) * 2;
            case EAttributeType::U32X3: return sizeof(uint32_t) * 3;
            case EAttributeType::U32X4: return sizeof(uint32_t) * 4;
            case EAttributeType::I32X1: return sizeof(int32_t) * 1;
            case EAttributeType::I32X2: return sizeof(int32_t) * 2;
            case EAttributeType::I32X3: return sizeof(int32_t) * 3;
            case EAttributeType::I32X4: return sizeof(int32_t) * 4;
            case EAttributeType::F32X1: return sizeof(float) * 1;
            case EAttributeType::F32X2: return sizeof(float) * 2;
            case EAttributeType::F32X3: return sizeof(float) * 3;
            case EAttributeType::F32X4: return sizeof(float) * 4;
            case EAttributeType::F64X1: return sizeof(double) * 1;
            case EAttributeType::F64X2: return sizeof(double) * 2;
            case EAttributeType::F64X3: return sizeof(double) * 3;
            case EAttributeType::F64X4: return sizeof(double) * 4;
            default: return UINT32_MAX;
            };
        };

        struct Attribute
        {
            
            explicit Attribute(EAttributeType type, std::string name)
                : type(type)
                , name(std::move(name))

            {
            }

            uint32_t       offset = 0;
            uint32_t       size;
            std::string    name;
            EAttributeType type;
        };

        explicit VertexBufferLayout(std::vector<Attribute> inAttributes)
            : attributes(inAttributes)
        {
            for (auto& atr : attributes)
            {
                atr.offset = stride;
                atr.size   = SizeOfAttributeType(atr.type);
                stride += atr.size;
            }
        }

        explicit VertexBufferLayout(std::initializer_list<Attribute> inAttributes)
            : attributes(inAttributes)
        {
            for (auto& atr : attributes)
            {
                atr.offset = stride;
                stride += SizeOfAttributeType(atr.type);
            }
        }

        uint32_t               stride = 0;
        std::vector<Attribute> attributes;
    };

    struct DepthStencilState
    {
		bool depthEnabled = true;
		bool stencilEnable = true;
    };
    
	struct RenderTargetLayout
	{
		uint32_t renderTargetColorAttachmentCount;
		EPixelFormat formats[8];
		EPixelFormat depthFormat;
	};
	
} // namespace PipelineStateDesc

struct GraphicsPipelineStateDesc
{
    IShaderBytecode* pVertexShader   = nullptr;
    IShaderBytecode* pPixelShader    = nullptr;
    IShaderBytecode* pDomainShader   = nullptr;
    IShaderBytecode* pHullShader     = nullptr;
    IShaderBytecode* pGeometryShader = nullptr;
    
    PipelineStateDesc::VertexBufferLayout vertexBufferLayout;
    // PipelineStateDesc::RasterizationState rasterizationState;
    // PipelineStateDesc::MultisampleState   multisampleState;
    PipelineStateDesc::DepthStencilState  depthStencilState;
    // PipelineStateDesc::ColorBlendState    colorBlendState;
	PipelineStateDesc::RenderTargetLayout renderTargetLayout;

	class IPipelineLayout* pPipelineLayout;
};

struct ComputePipelineStateDesc
{
    IShaderBytecode* pGeometryShader = nullptr;
	IPipelineLayout* pPipelineLayout;
};

class IPipelineState
{
public:
    virtual ~IPipelineState() = default;

};
using PipelineStatePtr = Unique<IPipelineState>;

} // namespace RHI
