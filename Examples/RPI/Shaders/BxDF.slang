
// namespace BxDF
// {

// TODO: move to constants.slang
static const float PI = 3.14159265359f;

struct BxDFContext
{
    float3 h;       // Halfway vector
    float nDotV;    // Normal dot View direction
    float nDotL;    // Normal dot Light direction
    float vDotL;    // View direction dot Light direction
    float nDotH;    // Normal dot Halfway vector
    float vDotH;    // View direction dot Halfway vector

    __init(float3 normal, float3 viewDir, float3 lightDir)
    {
        h = normalize(viewDir + lightDir);
        nDotV = dot(normal, viewDir);
        nDotL = dot(normal, lightDir);
        vDotL = dot(viewDir, lightDir);
        nDotH = dot(normal, h);
        vDotH = dot(viewDir, h);
    }
};

// Some common BxDF diffuse functions
float3 DiffuseLambertian(BxDFContext c, float3 diffuseColor)
{
    return diffuseColor * (1.0 / PI);
}

// float3 DiffuseBurley(BxDFContext c, float3 diffuseColor, float roughness)
// {
//     return {};
// }

// float3 DiffuseOrenNayar(BxDFContext c, float3 diffuseColor, float roughness)
// {
//     return {};
// }

// // Some common distriubtion functions

// float DistBlinn(BxDFContext c)
// {
//     return 0.0;
// }

// float DistBeckmann(BxDFContext c)
// {
//     return 0.0;
// }

// float DistGGX(BxDFContext c)
// {
//     return 0.0;
// }

// float DistAnsiGGX(BxDFContext c)
// {
//     return 0.0;
// }


// // Some common geo functions

// float GeoImplicit(BxDFContext c)
// {
// 	return 0.25;
// }

// float GeoNeumann(BxDFContext c)
// {
// 	return 1 / ( 4 * max( NoL, NoV ) );
// }

// float GeoKelemen(BxDFContext c)
// {
// 	// constant to prevent NaN
// 	return rcp( 4 * VoH * VoH + 1e-5);
// }

// float GeoSchlick(BxDFContext c)
// {
// 	float k = sqrt(a2) * 0.5;
// 	float Vis_SchlickV = NoV * (1 - k) + k;
// 	float Vis_SchlickL = NoL * (1 - k) + k;
// 	return 0.25 / ( Vis_SchlickV * Vis_SchlickL );
// }

// float GeoSmith(BxDFContext c)
// {
// 	float Vis_SmithV = NoV + sqrt( NoV * (NoV - NoV * a2) + a2 );
// 	float Vis_SmithL = NoL + sqrt( NoL * (NoL - NoL * a2) + a2 );
// 	return rcp( Vis_SmithV * Vis_SmithL );
// }

// float GeoSmithJointApprox(BxDFContext c)
// {
// 	float a = sqrt(a2);
// 	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
// 	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
// 	return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
// }

// float GeoSmithJoint(BxDFContext c)
// {
// 	float Vis_SmithV = NoL * sqrt(NoV * (NoV - NoV * a2) + a2);
// 	float Vis_SmithL = NoV * sqrt(NoL * (NoL - NoL * a2) + a2);
// 	return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
// }

// float GeoSmithJointAniso(BxDFContext c)
// {
// 	float Vis_SmithV = NoL * length(float3(ax * XoV, ay * YoV, NoV));
// 	float Vis_SmithL = NoV * length(float3(ax * XoL, ay * YoL, NoL));
// 	return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
// }

// // Some common sheen functions

// float3 F_None( float3 SpecularColor )
// {
// 	return SpecularColor;
// }

// // [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
// float3 F_Schlick( float3 SpecularColor, float VoH )
// {
// 	float Fc = Pow5( 1 - VoH );					// 1 sub, 3 mul
// 	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad

// 	// Anything less than 2% is physically impossible and is instead considered to be shadowing
// 	return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
// }

// float3 F_Schlick(float3 F0, float3 F90, float VoH)
// {
// 	float Fc = Pow5(1 - VoH);
// 	return F90 * Fc + (1 - Fc) * F0;
// }

// float3 F_AdobeF82(float3 F0, float3 F82, float VoH)
// {
// 	// [Kutz et al. 2021, "Novel aspects of the Adobe Standard Material" ]
// 	// See Section 2.3 (note the formulas in the paper do not match the code, the code is the correct version)
// 	// The constants below are derived by just constant folding the terms dependent on CosThetaMax=1/7
// 	const float Fc = Pow5(1 - VoH);
// 	const float K = 49.0 / 46656.0;
// 	float3 b = (K - K * F82) * (7776.0 + 9031.0 * F0);
// 	return saturate(F0 + Fc * ((1 - F0) - b * (VoH - VoH * VoH)));
// }

// float3 F_Fresnel( float3 SpecularColor, float VoH )
// {
// 	float3 SpecularColorSqrt = sqrt( clamp(SpecularColor, float3(0, 0, 0), float3(0.99, 0.99, 0.99) ) );
// 	float3 n = ( 1 + SpecularColorSqrt ) / ( 1 - SpecularColorSqrt );
// 	float3 g = sqrt( n*n + VoH*VoH - 1 );
// 	return 0.5 * Square( (g - VoH) / (g + VoH) ) * ( 1 + Square( ((g+VoH)*VoH - 1) / ((g-VoH)*VoH + 1) ) );
// }

// }
