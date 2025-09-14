#include <Shaders/GpuCommonStructs.h>

struct ShaderBindingTable
{
    ConstantBuffer<GPU::SceneView>                 sceneView;
    StructuredBuffer<GPU::DrawRequest>             drawRequests;
    StructuredBuffer<GPU::StaticMeshIndexed>       indexedMeshes;
    StructuredBuffer<float4x4>                     transforms;
    StructuredBuffer<GPU::MeshMaterialBindless>    materials;
    RWStructuredBuffer<U32>                        drawParametersCount;
    RWStructuredBuffer<GPU::DrawIndexedParameters> outDrawParameters;

    // Pass bind points
    Texture2D                                      pass_gBuffer_wsPosition;
    Texture2D                                      pass_gBuffer_normal;;
    Texture2D                                      pass_gBuffer_material;
    Texture2D                                      pass_gBuffer_depth;

    Texture2D                                      pass_lighting_input;

    RWTexture2D                                    pass_compose_output;

    Texture2D                                      bindlessTextures[];
};

ParameterBlock<ShaderBindingTable> bindings;

GPU::SceneView GetSceneView()
{
    return bindings.sceneView;
}

GPU::DrawRequest GetDrawRequest(uint idx)
{
    return bindings.drawRequests[idx];
}

GPU::StaticMeshIndexed GetIndexedMesh(uint idx)
{
    return bindings.indexedMeshes[idx];
}

float4x4 GetTransform(uint idx)
{
    return bindings.transforms[idx];
}

GPU::MeshMaterialBindless GetMaterial(uint idx)
{
    return bindings.materials[idx];
}

Texture2D GetBindlessTexture(uint idx)
{
    return bindings.bindlessTextures[idx];
}

bool PushDrawParameter(GPU::DrawRequest request)
{
    var currentIdnex = bindings.drawParametersCount[0];
    InterlockedAdd(bindings.drawParametersCount[0], 1);

    var mesh = bindings.indexedMeshes[request.meshId];

    GPU::DrawIndexedParameters outData;
    outData.indexCount = mesh.indexCount;
    outData.firstIndex = mesh.firstIndex;
    outData.vertexOffset = mesh.vertexOffset;
    outData.firstInstance = 0;
    outData.instanceCount = 1;
    bindings.outDrawParameters[currentIdnex] = outData;

    return false;
}

// float4 TransformModelToClip(float3 position, U32 modelID)
// {
//     var modelToWorld = GetTransform(modelID);
//     return mul(modelToWorld * bindings.sceneView.worldToClipMatrix, float4(position, 1));
// }

float4 TransfromWorldToClip(float3 position);