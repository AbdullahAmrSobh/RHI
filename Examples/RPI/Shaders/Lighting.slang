import BxDF;

// struct VertexInput
// {
//     float3 position;
//     float3 normal;
//     float2 uv;
// };

// struct FragmentInput
// {
//     float4 csPosition; // The clip space position of the fragment.
//     float3 position; // The position of the the fragment in world space.
//     float3 normal; // The normal vector of the fragment in world space.
//     float2 uv; // The texture coordinates of the fragment input.
// };

// // MaterialSurface is a struct that contains the material properties of a surface at a given point.
struct MaterialSurface
{
    float3 viewDirection; // The view direction at the point.
    float3 wsPosition; // The world space position of the point.
    float3 albedo; // The albedo color of the material.
    float3 normal; // The normal vector of the material.
    float metallic; // The metallic property of the material.
    float roughness; // The roughness property of the material.'
};

// static const uint32_t kMaxPointLights = 32; // Maximum number of point lights.
// static const uint32_t kMaxSpotLights = 32; // Maximum number of spot lights.

struct DirectionalLight
{
    float3 direction; // The direction of the directional light.
    float3 color; // The color of the directional light.
    float intensity;
};

struct PointLight
{
    float3 position; // The position of the point light.
    float radius; // The radius of the point light.
    float3 color; // The color of the point light.
    float intensity; // The intensity of the point light.
};

struct SpotLight
{
    float3 position; // The position of the spot light.
    float3 direction; // The direction of the spot light.
    float radius; // The radius of the spot light.
    float angle; // The angle of the spot light.
    float3 color; // The color of the spot light.
    float intensity; // The intensity of the spot light.
};

// struct Scene
// {
//     float4x4 worldToViewMatrix; // The world-to-view matrix transforms from world space to view space.
//     float4x4 viewToClipMatrix; // The view-to-clip matrix transforms from view space to clip space.
//     float4x4 viewToWorldMatrix; // The view-to-world matrix transforms from view space to world space.
//     float3 cameraPosition; // The position of the camera in world space.
//     float3 ambientIntensity; // The intensity of the ambient light in the scene.
//     DirectionalLight directionalLight; // The directional light in the scene.
//     uint32_t numPointLights; // The number of point lights in the scene.
//     PointLight pointLights[kMaxPointLights]; // The point lights in the scene.
//     uint32_t numSpotLights; // The number of spot lights in the scene.
//     SpotLight spotLights[kMaxSpotLights]; // The spot lights in the scene.
// };

// struct Material
// {
//     float3 albedo; // The albedo color of the material.
//     uint32_t albedoMapIndex; // The index of the albedo map texture.
//     uint32_t normalMapIndex; // The index of the normal map texture.
//     float metallic; // The metallic property of the material.
//     float roughness; // The roughness property of the material.
//     uint32_t metallicRoughnessMapIndex; // The index of the metallic-roughness map texture.
// };

// struct Model
// {
//     float4x3 modelToWorldMatrix; // The model-to-world matrix transforms from model space to world space.
//     Material material; // The material of the model.
// };

// MaterialSurface getMaterialSurface(FragmentInput input)
// {
//     MaterialSurface materialSurface;
//     materialSurface.viewDirection = normalize(bindGroup0.sceneBuffer.cameraPosition - input.position);
//     materialSurface.wsPosition = input.position;
//     materialSurface.albedo = bindGroup0.modelBuffer.material.albedo;
//     materialSurface.normal = normalize(input.normal);
//     materialSurface.metallic = bindGroup0.modelBuffer.material.metallic;
//     materialSurface.roughness = bindGroup0.modelBuffer.material.roughness;
//     if (bindGroup0.modelBuffer.material.albedoMapIndex != 0xFFFFFFFF)
//     {
//         materialSurface.albedo = bindGroup0.textures[bindGroup0.modelBuffer.material.albedoMapIndex].Sample(bindGroup0.sampler, input.uv).xyz;
//     }
//     if (bindGroup0.modelBuffer.material.normalMapIndex != 0xFFFFFFFF)
//     {
//         materialSurface.normal = normalize(bindGroup0.textures[bindGroup0.modelBuffer.material.normalMapIndex].Sample(bindGroup0.sampler, input.uv).xyz * 2.0f - 1.0f);
//     }
//     if (bindGroup0.modelBuffer.material.metallicRoughnessMapIndex != 0xFFFFFFFF)
//     {
//         float4 metallicRoughness = bindGroup0.textures[bindGroup0.modelBuffer.material.metallicRoughnessMapIndex].Sample(bindGroup0.sampler, input.uv);
//         materialSurface.metallic = metallicRoughness.x;
//         materialSurface.roughness = metallicRoughness.y;
//     }
//     return materialSurface;
// }

// float3 evaluateBRDF(MaterialSurface materialSurface, float3 lightDirection, float3 viewDirection)
// {
//     float3 halfVector = normalize(lightDirection + viewDirection);
//     float nDotL = saturate(dot(materialSurface.normal, lightDirection));
//     float nDotV = saturate(dot(materialSurface.normal, viewDirection));
//     float nDotH = saturate(dot(materialSurface.normal, halfVector));
//     float vDotH = saturate(dot(viewDirection, halfVector));
//     float roughness = materialSurface.roughness;
//     float3 f0 = float3(0.04f); // Minimum reflectance at normal incidence.
//     float3 f90 = float3(1.0f); // Maximum reflectance at glancing angle.
//     float3 f = fresnelSchlickRoughness(f0, vDotH, f90);
//     float d = ggxDistribution(materialSurface.normal, halfVector, roughness);
//     float g = geometrySmith(nDotV, nDotL, roughness);
//     return f * d * g / (4.0 * nDotL * nDotV);
// }

// float3 AccumateLights(MaterialSurface materialSurface)
// {
//     // Evaluate the directional light.
//     float3 lightDirection = -bindGroup0.sceneBuffer.directionalLight.direction;
//     float3 lightColor = bindGroup0.sceneBuffer.directionalLight.color;
//     float nDotLDirectional = saturate(dot(materialSurface.normal, lightDirection));
//     float3 color = evaluateBRDF(materialSurface, lightDirection, materialSurface.viewDirection) * lightColor * nDotLDirectional;

//     // Evaluate the point lights.
//     for (uint32_t i = 0; i < bindGroup0.sceneBuffer.numPointLights; i++)
//     {
//         // Evaluate the point light.
//         PointLight pointLight = bindGroup0.sceneBuffer.pointLights[i];
//         float3 lightDirection = normalize(pointLight.position - materialSurface.wsPosition);
//         float3 lightColor = pointLight.color * pointLight.intensity;
//         float distance = length(pointLight.position - materialSurface.wsPosition);
//         float attenuation = 1.0 / (1.0 + distance * distance);
//         float nDotLPoint = saturate(dot(materialSurface.normal, lightDirection));
//         color += evaluateBRDF(materialSurface, lightDirection, materialSurface.viewDirection) * lightColor * nDotLPoint * attenuation;
//     }

//     // Evaluate the spot lights.
//     for (uint32_t i = 0; i < bindGroup0.sceneBuffer.numSpotLights; i++)
//     {
//         // Evaluate the spot light.
//         SpotLight spotLight = bindGroup0.sceneBuffer.spotLights[i];
//         float3 lightDirection = normalize(spotLight.position - materialSurface.wsPosition);
//         float3 lightColor = spotLight.color * spotLight.intensity;
//         float distance = length(spotLight.position - materialSurface.wsPosition);
//         float attenuation = 1.0 / (1.0 + distance * distance);
//         float nDotLSpot = saturate(dot(materialSurface.normal, lightDirection));
//         float spotFactor = saturate((dot(-lightDirection, spotLight.direction) - cos(spotLight.angle)) / (1.0 - cos(spotLight.angle)));
//         color += evaluateBRDF(materialSurface, lightDirection, materialSurface.viewDirection) * lightColor * nDotLSpot * attenuation * spotFactor;
//     }

//     // Exposure tone mapping.

//     float exposure = 1.5;

//     color = 1.0 - exp(-color * exposure);

//     return color;
// }
