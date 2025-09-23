#include "Shaders/GpuCommonStructs.h"

struct CullParams
{
    float4x4 viewProjection;
    float2   viewportSize;
    float2   cameraPosition;

    uint drawRequestCount;
    uint indexedMeshCount;

    StructuredBuffer<GPU::DrawRequest>         drawRequests;
    StructuredBuffer<GPU::DrawIndexedParameters>         indexedMeshes;

    // OUTPUT
    RWStructuredBuffer<GPU::SceneRenderable>   renderables;
    RWStructuredBuffer<uint>                   drawCount;
    RWStructuredBuffer<GPU::DrawIndexedParameters> drawIndirectArgs;
};

ParameterBlock<CullParams> params;

[[shader("compute"), numthreads(64, 1, 1)]]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID,
            uint3 groupThreadID    : SV_GroupThreadID,
            uint3 groupID          : SV_GroupID)
{
    uint idx = dispatchThreadID.x;
    if (idx >= params.drawRequestCount)
        return;

    // Reset counter once per dispatch (thread 0 only)
    if (idx == 0)
    {
        params.drawCount[0] = 0;
        AllMemoryBarrierWithGroupSync();
    }

    GPU::DrawRequest req = params.drawRequests[idx];
    GPU::DrawIndexedParameters mesh = params.indexedMeshes[req.meshID];

    // Reserve a slot in the output
    uint outIndex;
    InterlockedAdd(params.drawCount[0], 1, outIndex);

    // Fill indirect draw args
    GPU::DrawIndexedParameters args;
    args.indexCount     = mesh.indexCount;
    args.instanceCount  = 1;
    args.firstIndex     = mesh.firstIndex;
    args.vertexOffset   = mesh.vertexOffset;
    args.firstInstance  = outIndex;
    params.drawIndirectArgs[outIndex] = args;

    // Fill renderable info (minimal for now)
    GPU::SceneRenderable renderable;
    renderable.instanceID = req.instanceID;
    renderable.materialID = req.materialID;
    renderable.meshID     = req.meshID;
    params.renderables[outIndex] = renderable;
}
