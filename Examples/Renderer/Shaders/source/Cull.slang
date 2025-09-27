#include "Shaders/GpuCommonStructs.h"

struct CullParams
{
    StructuredBuffer<GPU::StaticMeshIndexed>   staticMeshOffsets;
    StructuredBuffer<GPU::DrawRequest>         drawRequests;
    RWStructuredBuffer<uint>                       drawCount;
    RWStructuredBuffer<GPU::DrawIndexedParameters> drawIndirectArgs;
};

ParameterBlock<CullParams> params;

[[shader("compute"), numthreads(64, 1, 1)]]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID,
            uint3 groupThreadID    : SV_GroupThreadID,
            uint3 groupID          : SV_GroupID)
{
    uint idx = dispatchThreadID.x;
    // if (idx >= params.drawRequestCount)
    //     return;

    // Reset counter once per dispatch (thread 0 only)
    if (idx == 0)
    {
        params.drawCount[0] = 0;
        AllMemoryBarrierWithGroupSync();
    }

    // GPU::DrawRequest req = params.drawRequests[idx];
    // GPU::DrawIndexedParameters mesh = params.indexedMeshes[req.meshID];

    // Reserve a slot in the output
    uint outIndex;
    InterlockedAdd(params.drawCount[0], 1, outIndex);


    let staticMesh = params.staticMeshOffsets[outIndex];

    // Fill indirect draw args
    GPU::DrawIndexedParameters args;
    args.indexCount     = staticMesh.indexCount;
    args.instanceCount  = 1;
    args.firstIndex     = staticMesh.firstIndex;
    args.vertexOffset   = staticMesh.vertexOffset;
    args.firstInstance  = 0;
    params.drawIndirectArgs[outIndex] = args;

    // // Fill renderable info (minimal for now)
    // GPU::SceneRenderable renderable;
    // renderable.instanceID = req.instanceID;
    // renderable.materialID = req.materialID;
    // renderable.meshID     = req.meshID;
    // params.renderables[outIndex] = renderable;
}
