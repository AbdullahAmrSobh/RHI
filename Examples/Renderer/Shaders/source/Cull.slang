import ShaderBindingTable;

[[shader("compute"), numthreads(1, 1, 1)]]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{
    var request = bindings.drawRequests[threadId.x];
    var mesh    = bindings.indexedMeshes[request.meshId];

    if (threadId.x == 0)
    {
        bindings.drawParametersCount[0] = 0;
        AllMemoryBarrier();
    }

    InterlockedAdd(bindings.drawParametersCount[0], 1);

    GPU::DrawIndexedParameters outData;
    outData.indexCount = mesh.indexCount;
    outData.firstIndex = mesh.firstIndex;
    outData.vertexOffset = mesh.vertexOffset;
    outData.firstInstance = 0;
    outData.instanceCount = 1;
    bindings.outDrawParameters[threadId.x] = outData;
}