#include "Shaders/GpuCommonStructs.h"
struct CullParams
{
    uint32_t drawCount;

    StructuredBuffer<GPU::StaticMeshIndexed>       staticMeshOffsets;
    StructuredBuffer<GPU::DrawRequest>             drawRequests;
    RWStructuredBuffer<uint>                       drawCountOut;
    RWStructuredBuffer<GPU::DrawIndexedParameters> drawIndirectArgs;
};

ParameterBlock<CullParams> params;

[[shader("compute"), numthreads(64, 1, 1)]]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID,
            uint3 groupThreadID    : SV_GroupThreadID,
            uint3 groupID          : SV_GroupID)
{
    uint idx = dispatchThreadID.x;

    // Reset once at the beginning
    if (idx == 0)
    {
        params.drawCountOut[0] = 0;
    }
    GroupMemoryBarrierWithGroupSync();

    if (idx >= params.drawCount)
        return;

    // Get request + mesh info
    GPU::DrawRequest req      = params.drawRequests[idx];
    GPU::StaticMeshIndexed sm = params.staticMeshOffsets[req.meshID];

    // Reserve a slot in the output
    uint outIndex;
    InterlockedAdd(params.drawCountOut[0], 1, outIndex);

    // Fill indirect draw args
    GPU::DrawIndexedParameters args;
    args.indexCount    = sm.indexCount;
    args.instanceCount = 1;
    args.firstIndex    = sm.firstIndex;
    args.vertexOffset  = sm.vertexOffset;
    args.firstInstance = idx;
    params.drawIndirectArgs[outIndex] = args;

    printf("di: idx: %i, meshID: %i (sm.indexCount: %i, sm.firstIndex: %i, sm.vertexOffset: %i) ",
        idx, req.meshID,
        sm.indexCount, sm.firstIndex, sm.vertexOffset);
}
