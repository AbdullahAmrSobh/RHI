import Public.GpuScene;

// ============================================================================
// Constants
// ============================================================================

static const float PI = 3.141592653589793;
static const float MIN_F0 = 0.04;

// ============================================================================
// Utility Functions
// ============================================================================

/// Squares the input value.
/// @param value The value to be squared.
/// @returns The square of the input value.
float square(float value)
{
    return value * value;
}

/// Computes the fifth power of the input value.
/// @param value The value to be raised to the fifth power.
/// @returns The input value raised to the power of five.
float pow5(float value)
{
    return pow(value, 5);
}

// ============================================================================
// Data Structures
// ============================================================================

/// Represents the clear coat properties of a surface.
struct ClearCoatSurface
{
    float weight;    ///< The intensity of the clear coat layer.
    float roughness; ///< The roughness of the clear coat layer.
};

/// Contains settings for anisotropic materials.
struct AnisotropicSettings
{
    float3 tangent;     ///< Tangent vector for anisotropy direction.
    float3 bitangent;   ///< Bitangent vector orthogonal to the tangent.
    float2 roughnessA;  ///< Roughness values along the tangent and bitangent.
};

/// Defines transmission properties for translucent materials.
struct TransmissionSettings
{
    float3 tint;               ///< Color tint of the transmitted light.
    float thickness;           ///< Thickness of the material.
    float4 transmissionParams; ///< Additional parameters for transmission.
    float3 scatterDistance;    ///< Distance over which light scatters inside the material.
};

/// Represents the Bidirectional Scattering Distribution Function components.
struct BSDF
{
    float3 diffuse;      ///< Diffuse reflection component.
    float3 specular;     ///< Specular reflection component.
    float3 transmission; ///< Transmission component (for transparent materials).
};

/// Describes the surface properties at a shading point.
struct Surface
{
    float3 viewDir;       ///< Direction from the surface point to the viewer.
    float3 position;      ///< World-space position of the surface point.
    float3 normal;        ///< Normal vector at the surface point.
    float3 vertexNormal;  ///< Original vertex normal before any modifications.
    float metallic;       ///< Metallic property of the material [0, 1].
    float roughnessA;     ///< Primary roughness parameter.
    float roughnessA2;    ///< Secondary roughness parameter (often roughnessA squared).
    float3 albedo;        ///< Base color of the material.
    float3 diffuseColor;  ///<
    float3 specularColor; ///<

    __init(float3 position, float3 normal, float3 vertexNormal, float3 albedo, float metallic, float roughness)
    {
        this.viewDir = GetViewDirection();
        this.position = position;
        this.normal = normal;
        this.vertexNormal = vertexNormal;
        this.metallic = metallic;
        this.roughnessA = roughnessToPerceptualRoughness(roughness);
        this.roughnessA2 = this.roughnessA * this.roughnessA;
        this.albedo = albedo;
        this.diffuseColor = computeDiffuseColor(this.albedo, this.metallic);
        this.specularColor = computeF0(this.albedo, this.metallic, MIN_F0);
        // this.specularColor = albedo.rgb * metallic;
    }
};

// ============================================================================
// Remapping functions
// ============================================================================

/// Computes the diffuse color component for a material.
/// In a metallic workflow, the diffuse color is derived from the base color
/// and is diminished as the metallic property increases.
/// @param baseColor The base color of the material, including alpha.
/// @param metallic The metallic factor of the material [0, 1].
/// @returns The computed diffuse color.
float3 computeDiffuseColor(float3 baseColor, float metallic)
{
    // For metallic materials, the diffuse component is reduced.
    return baseColor.rgb * (1.0 - metallic);
}

half DielectricSpecularToF0(half Specular)
{
	return half(0.08f * Specular);
}

/// Computes the Fresnel reflectance at normal incidence (F0) for a material.
/// This function blends between the base color and a default reflectance
/// based on the metallic property.
/// @param baseColor The base color of the material, including alpha.
/// @param metallic The metallic factor of the material [0, 1].
/// @param reflectance The base reflectance value for non-metallic surfaces.
/// @returns The computed F0 reflectance value.
float3 computeF0(float3 baseColor, float metallic, float reflectance)
{
    return lerp(DielectricSpecularToF0(half(reflectance)).xxx, baseColor, 1.0 - metallic);
}

/// Computes the default Fresnel reflectance at normal incidence (F0)
/// for a dielectric (non-metallic) material based on its reflectance value.
/// @param reflectance The base reflectance value.
/// @returns The computed dielectric F0 value.
float computeDielectricF0(float reflectance)
{
    // Dielectric F0 is approximately 4% reflectance (0.04) squared.
    return 0.16 * reflectance * reflectance;
}

/// Converts a glossiness value to roughness.
/// Glossiness and roughness are inversely related.
/// @param glossiness The glossiness value [0, 1].
/// @returns The corresponding roughness value.
float computeRoughnessFromGlossiness(float glossiness)
{
    // Roughness is the inverse of glossiness.
    return 1.0 - glossiness;
}

/// Converts perceptual roughness to physical roughness.
/// Perceptual roughness is often stored in textures and needs to be squared
/// to obtain the physical roughness used in shading calculations.
/// @param perceptualRoughness The perceptual roughness value [0, 1].
/// @returns The physical roughness value.
float perceptualRoughnessToRoughness(float perceptualRoughness)
{
    // Physical roughness is the square of perceptual roughness.
    return perceptualRoughness * perceptualRoughness;
}

/// Converts physical roughness to perceptual roughness.
/// This is the inverse of the perceptualRoughnessToRoughness function.
/// @param roughness The physical roughness value [0, 1].
/// @returns The perceptual roughness value.
float roughnessToPerceptualRoughness(float roughness)
{
    // Perceptual roughness is the square root of physical roughness.
    return sqrt(roughness);
}

/// Converts the index of refraction (IOR) to Fresnel reflectance at normal incidence (F0).
/// This function uses the relationship between IOR and F0 for dielectrics.
/// @param transmittedIor The IOR of the transmitted medium.
/// @param incidentIor The IOR of the incident medium.
/// @returns The computed F0 value.
float iorToF0(float transmittedIor, float incidentIor)
{
    // Computes F0 using the Fresnel equation for dielectrics.
    return square((transmittedIor - incidentIor) / (transmittedIor + incidentIor));
}

/// Converts Fresnel reflectance at normal incidence (F0) to the index of refraction (IOR).
/// This is the inverse of the iorToF0 function.
/// @param f0 The Fresnel reflectance at normal incidence.
/// @returns The computed IOR.
float f0ToIor(float f0)
{
    // Computes IOR from F0 using the inverse of the Fresnel equation.
    float r = sqrt(f0);
    return (1.0 + r) / (1.0 - r);
}

// ============================================================================
// BRDF Helper Functions
// ============================================================================

/// Computes the GGX Normal Distribution Function (NDF).
/// @param a2 Squared roughness parameter.
/// @param NdotH Dot product of the surface normal and the half-vector.
/// @returns The GGX NDF value.
float DistGGX(float a2, float NdotH)
{
    float b = (NdotH * a2 - NdotH) * NdotH + 1.0;
    return a2 / (PI * b * b);
}

/// Computes the Kelemen visibility function.
/// @param vDotH Dot product of the view direction and the half-vector.
/// @returns The visibility term based on Kelemen's model.
float VisKelemen(float vDotH)
{
    return rcp(4 * vDotH * vDotH + 1e-5);
}

/// Computes the Smith visibility function using the GGX model.
/// @param a2 Squared roughness parameter.
/// @param nDotV Dot product of the normal and the view direction.
/// @param nDotL Dot product of the normal and the light direction.
/// @returns The visibility term based on Smith's model.
float VisSmith(float a2, float nDotV, float nDotL)
{
    float V = nDotV * sqrt(a2 + (1.0 - a2) * nDotV * nDotV);
    float L = nDotL * sqrt(a2 + (1.0 - a2) * nDotL * nDotL);
    return 0.5 / (V + L);
}

/// Computes the Fresnel term using Schlick's approximation.
/// @param SpecularColor The base specular color.
/// @param VoH Dot product of the view direction and the half-vector.
/// @returns The Fresnel reflection coefficient.
float3 FresnelSchlick(float3 SpecularColor, float VoH)
{
    // Compute Fresnel falloff
    // Apply Schlick's approximation: F = F0 + (1 - F0) * (1 - cos(theta))^5
    return SpecularColor + (1.0 - SpecularColor) * pow5(1.0 - VoH);
}

/// Computes the Lambertian diffuse reflection.
/// @param diffuseColor The base diffuse color.
/// @returns The Lambertian BRDF value.
float3 DiffuseLambert(float3 diffuseColor)
{
    return diffuseColor * (1.0f / PI);
}

/// Computes the Burley diffuse reflection.
/// @param diffuseColor The base diffuse color.
/// @param roughness Surface roughness.
/// @param nDotV Dot product of the normal and the view direction.
/// @param nDotL Dot product of the normal and the light direction.
/// @param vDotH Dot product of the view direction and the half-vector.
/// @returns The Burley diffuse BRDF value.
float3 DiffuseBurley(float3 diffuseColor, float roughness, float nDotV, float nDotL, float vDotH)
{
    float FD90 = 0.5 + 2 * vDotH * vDotH * roughness;
    float FdV = 1 + (FD90 - 1) * pow(1 - nDotV, 5);
    float FdL = 1 + (FD90 - 1) * pow(1 - nDotL, 5);
    return diffuseColor * ((1 / PI) * FdV * FdL);
}

/// Computes the specular reflection component using the GGX microfacet model.
/// @param specularColor The base specular color (F0).
/// @param roughness The surface roughness parameter.
/// @param nDotV Dot product of the normal and view direction.
/// @param nDotL Dot product of the normal and light direction.
/// @param nDotH Dot product of the normal and half-vector.
/// @param vDotH Dot product of the view direction and half-vector.
/// @returns The specular reflection component.
float3 SpecularGGX(float3 specularColor, float roughnessA2, float nDotV, float nDotL, float nDotH, float vDotH)
{
    let D = DistGGX(roughnessA2, nDotH);
    let V = VisSmith(roughnessA2, nDotV, nDotL);
    let F = FresnelSchlick(specularColor, vDotH);
    return (D * V) * F;
}

// ============================================================================
// BSDF Evaluation Function
// ============================================================================

/// Evaluates the BSDF for a given surface and light interaction.
/// @param surface The surface properties at the shading point.
/// @param lightData The incoming light data.
/// @returns The computed BSDF components.
// BSDF EvaluateBSDF(const Surface surface, const LightData lightData)
// {
//      TODO
// }
