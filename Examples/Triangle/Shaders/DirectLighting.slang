import Public.GpuScene;
import BxDF;

struct DirectLighting
{
    float3 diffuse;
    float3 specular;
    float3 transmission;
};

void ApplyDirectionalLight(DirectionalLight light, Surface surface, inout DirectLighting lighting)
{
    let nDotL = saturate(dot(surface.normal, light.direction));
    let h = normalize(light.direction + surface.viewDir);
    let nDotH = saturate(dot(surface.normal, h));

    let diffuse = light.color * nDotL;
    let specular = light.color * pow(nDotH, surface.roughnessA);

    lighting.diffuse += surface.diffuseColor * diffuse;
    lighting.specular += surface.specularColor * specular;
}

void ApplyPointLight(PointLight light, Surface surface, inout DirectLighting lighting)
{
    // var l = light.position - surface.position;
    // let distance = length(l);
    // l /= distance;

    // let nDotL = saturate(dot(surface.normal, l));
    // let h = normalize(l + surface.viewDir);
    // let nDotH = saturate(dot(surface.normal, h));

    // let attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * distance + light.quadraticAttenuation * distance * distance);
    // let diffuse = light.color * nDotL * attenuation;
    // let specular = light.color * pow(nDotH, surface.roughnessA) * attenuation;

    // lighting.diffuse += surface.diffuseColor * diffuse;
    // lighting.specular += surface.specularColor * specular;
}

void ApplySpotLight(SpotLight light, Surface surface, inout DirectLighting lighting)
{
    // var l = light.position - surface.position;
    // let distance = length(l);
    // l /= distance;

    // let nDotL = saturate(dot(surface.normal, l));
    // let h = normalize(l + surface.viewDir);
    // let nDotH = saturate(dot(surface.normal, h));

    // let attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * distance + light.quadraticAttenuation * distance * distance);
    // let spotFactor = saturate(dot(-l, light.direction));
    // let spot = pow(spotFactor, light.spotExponent);
    // attenuation *= spot;

    // let diffuse = light.color * nDotL * attenuation;
    // let specular = light.color * pow(nDotH, surface.roughnessA) * attenuation;

    // lighting.diffuse += surface.diffuseColor * diffuse;
    // lighting.specular += surface.specularColor * specular;
}

void ApplyAreaLight(AreaLight light, Surface surface, inout DirectLighting lighting)
{
    // TODO
}

// DirectLighting AccumulateSceneLights(Surface surface)
// {
//     DirectLighting lighting;

//     // Directional Lighting
//     for (uint i = 0; i < scene.sceneView.directionalLightsCount; i++)
//     {
//         ApplyDirectionalLight(scene.directionalLights[i], surface, lighting);
//     }

//     // Point Lights
//     for (uint i = 0; i < scene.sceneView.pointLightsCount; i++)
//     {
//         let light = scene.pointLights[i];
//         ApplyPointLight(light, surface, lighting);
//     }

//     // Spot Lights
//     for (uint i = 0; i < scene.sceneView.spotLightsCount; i++)
//     {
//         let light = scene.spotLights[i];
//         ApplySpotLight(light, surface, lighting);
//     }

//     // Area Lights
//     for (uint i = 0; i < scene.sceneView.areaLightsCount; i++)
//     {
//         let light = scene.areaLights[i];
//         ApplyAreaLight(light, surface, lighting);
//     }

//     return lighting;
// }