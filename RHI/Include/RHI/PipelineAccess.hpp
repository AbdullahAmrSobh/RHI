#pragma once

#include <TL/Flags.hpp>

namespace RHI
{
    enum class PipelineStage
    {
        None                          = 0 << 0,
        TopOfPipe                     = 1 << 0,
        DrawIndirect                  = 1 << 1,
        VertexInput                   = 1 << 2,
        VertexShader                  = 1 << 3,
        TessellationControlShader     = 1 << 4,
        TessellationEvaluationShader  = 1 << 5,
        PixelShader                   = 1 << 6,
        EarlyFragmentTests            = 1 << 7,
        LateFragmentTests             = 1 << 8,
        ColorAttachmentOutput         = 1 << 9,
        ComputeShader                 = 1 << 10,
        Transfer                      = 1 << 12,
        BottomOfPipe                  = 1 << 13,
        Host                          = 1 << 14,
        AllGraphics                   = 1 << 15,
        AllCommands                   = 1 << 16,
        Copy                          = 1 << 17,
        Resolve                       = 1 << 18,
        Blit                          = 1 << 19,
        Clear                         = 1 << 20,
        IndexInput                    = 1 << 21,
        VertexAttributeInput          = 1 << 22,
        PreRasterizationShaders       = 1 << 23,
        TransformFeedback             = 1 << 24,
        ConditionalRendering          = 1 << 25,
        FragmentShadingRateAttachment = 1 << 26,
        AccelerationStructureBuild    = 1 << 27,
        RayTracingShader              = 1 << 28,
        TaskShader                    = 1 << 29,
        MeshShader                    = 1 << 30,
        AccelerationStructureCopy     = 1 << 31,
    };

    TL_DEFINE_FLAG_OPERATORS(PipelineStage);

    enum class Access
    {
        None      = 0,
        Read      = 1 << 0,
        Write     = 1 << 1,
        ReadWrite = Read | Write,
    };

    TL_DEFINE_FLAG_OPERATORS(Access);
} // namespace RHI