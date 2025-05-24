#include "Public/GpuScene.h"

// Input buffers

StructuredBuffer<GpuScene::StaticMeshIndexed>    indexedMeshResources;
StructuredBuffer<GpuScene::StaticMeshNonIndexed> nonIndexedMeshResources;
StructuredBuffer<GpuScene::MeshMaterialBindless> materialResources;
StructuredBuffer<GpuScene::MeshUniform>          modelUniforms;
StructuredBuffer<GpuScene::DrawRequest>          drawRequests;

RWStructuredBuffer<GpuScene::MeshUniform>           outModelUniforms;
RWStructuredBuffer<GpuScene::DrawIndexedParameters> drawIndexedResources;
RWStructuredBuffer<GpuScene::DrawParameters>        drawNonIndexedResources;

void StructuredBufferPushBack<T>(RWStructuredBuffer<T> buffer, T value)
{
    var count = 1;
    buffer[count] = value;
}

// Output buffer
void ProcessIndexedDrawRequest(U32 drawRequestId)
{
    var drawRequest  = drawRequests[drawRequestId];
    var mesh         = indexedMeshResources[drawRequest.meshId];
    var material     = materialResources[drawRequest.materialId];
    var modelUniform = modelUniforms[drawRequest.uniformId];

    GpuScene::DrawIndexedParameters parameters;

    StructuredBufferPushBack(drawIndexedResources, parameters);
}

[[shader("compute"), numthreads(1, 1, 1)]]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{
    for (U32 i = 0; i < 1; i++)
    {
        var drawRequest = drawRequests[i];
        ProcessIndexedDrawRequest(i);
    }
}