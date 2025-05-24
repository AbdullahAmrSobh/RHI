#include "Public/GpuScene.h"

ConstantBuffer<GpuScene::SceneView>              sceneView;
StructuredBuffer<GpuScene::MeshMaterialBindless> materialResources;
StructuredBuffer<GpuScene::MeshUniform>          modelUniforms;
